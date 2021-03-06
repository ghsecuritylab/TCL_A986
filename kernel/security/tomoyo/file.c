

#include "common.h"
#include <linux/slab.h>

/* Keyword array for single path operations. */
static const char *tomoyo_path_keyword[TOMOYO_MAX_PATH_OPERATION] = {
	[TOMOYO_TYPE_READ_WRITE] = "read/write",
	[TOMOYO_TYPE_EXECUTE]    = "execute",
	[TOMOYO_TYPE_READ]       = "read",
	[TOMOYO_TYPE_WRITE]      = "write",
	[TOMOYO_TYPE_CREATE]     = "create",
	[TOMOYO_TYPE_UNLINK]     = "unlink",
	[TOMOYO_TYPE_MKDIR]      = "mkdir",
	[TOMOYO_TYPE_RMDIR]      = "rmdir",
	[TOMOYO_TYPE_MKFIFO]     = "mkfifo",
	[TOMOYO_TYPE_MKSOCK]     = "mksock",
	[TOMOYO_TYPE_MKBLOCK]    = "mkblock",
	[TOMOYO_TYPE_MKCHAR]     = "mkchar",
	[TOMOYO_TYPE_TRUNCATE]   = "truncate",
	[TOMOYO_TYPE_SYMLINK]    = "symlink",
	[TOMOYO_TYPE_REWRITE]    = "rewrite",
	[TOMOYO_TYPE_IOCTL]      = "ioctl",
	[TOMOYO_TYPE_CHMOD]      = "chmod",
	[TOMOYO_TYPE_CHOWN]      = "chown",
	[TOMOYO_TYPE_CHGRP]      = "chgrp",
	[TOMOYO_TYPE_CHROOT]     = "chroot",
	[TOMOYO_TYPE_MOUNT]      = "mount",
	[TOMOYO_TYPE_UMOUNT]     = "unmount",
};

/* Keyword array for double path operations. */
static const char *tomoyo_path2_keyword[TOMOYO_MAX_PATH2_OPERATION] = {
	[TOMOYO_TYPE_LINK]    = "link",
	[TOMOYO_TYPE_RENAME]  = "rename",
	[TOMOYO_TYPE_PIVOT_ROOT] = "pivot_root",
};

void tomoyo_put_name_union(struct tomoyo_name_union *ptr)
{
	if (!ptr)
		return;
	if (ptr->is_group)
		tomoyo_put_path_group(ptr->group);
	else
		tomoyo_put_name(ptr->filename);
}

bool tomoyo_compare_name_union(const struct tomoyo_path_info *name,
			       const struct tomoyo_name_union *ptr)
{
	if (ptr->is_group)
		return tomoyo_path_matches_group(name, ptr->group, 1);
	return tomoyo_path_matches_pattern(name, ptr->filename);
}

static bool tomoyo_compare_name_union_pattern(const struct tomoyo_path_info
					      *name,
					      const struct tomoyo_name_union
					      *ptr, const bool may_use_pattern)
{
	if (ptr->is_group)
		return tomoyo_path_matches_group(name, ptr->group,
						 may_use_pattern);
	if (may_use_pattern || !ptr->filename->is_patterned)
		return tomoyo_path_matches_pattern(name, ptr->filename);
	return false;
}

const char *tomoyo_path2keyword(const u8 operation)
{
	return (operation < TOMOYO_MAX_PATH_OPERATION)
		? tomoyo_path_keyword[operation] : NULL;
}

const char *tomoyo_path22keyword(const u8 operation)
{
	return (operation < TOMOYO_MAX_PATH2_OPERATION)
		? tomoyo_path2_keyword[operation] : NULL;
}

static bool tomoyo_strendswith(const char *name, const char *tail)
{
	int len;

	if (!name || !tail)
		return false;
	len = strlen(name) - strlen(tail);
	return len >= 0 && !strcmp(name + len, tail);
}

static struct tomoyo_path_info *tomoyo_get_path(struct path *path)
{
	int error;
	struct tomoyo_path_info_with_data *buf = kzalloc(sizeof(*buf),
							 GFP_NOFS);

	if (!buf)
		return NULL;
	/* Reserve one byte for appending "/". */
	error = tomoyo_realpath_from_path2(path, buf->body,
					   sizeof(buf->body) - 2);
	if (!error) {
		buf->head.name = buf->body;
		tomoyo_fill_path_info(&buf->head);
		return &buf->head;
	}
	kfree(buf);
	return NULL;
}

static int tomoyo_update_path2_acl(const u8 type, const char *filename1,
				   const char *filename2,
				   struct tomoyo_domain_info *const domain,
				   const bool is_delete);
static int tomoyo_update_path_acl(const u8 type, const char *filename,
				  struct tomoyo_domain_info *const domain,
				  const bool is_delete);

LIST_HEAD(tomoyo_globally_readable_list);

static int tomoyo_update_globally_readable_entry(const char *filename,
						 const bool is_delete)
{
	struct tomoyo_globally_readable_file_entry *ptr;
	struct tomoyo_globally_readable_file_entry e = { };
	int error = is_delete ? -ENOENT : -ENOMEM;

	if (!tomoyo_is_correct_path(filename, 1, 0, -1))
		return -EINVAL;
	e.filename = tomoyo_get_name(filename);
	if (!e.filename)
		return -ENOMEM;
	if (mutex_lock_interruptible(&tomoyo_policy_lock))
		goto out;
	list_for_each_entry_rcu(ptr, &tomoyo_globally_readable_list, list) {
		if (ptr->filename != e.filename)
			continue;
		ptr->is_deleted = is_delete;
		error = 0;
		break;
	}
	if (!is_delete && error) {
		struct tomoyo_globally_readable_file_entry *entry =
			tomoyo_commit_ok(&e, sizeof(e));
		if (entry) {
			list_add_tail_rcu(&entry->list,
					  &tomoyo_globally_readable_list);
			error = 0;
		}
	}
	mutex_unlock(&tomoyo_policy_lock);
 out:
	tomoyo_put_name(e.filename);
	return error;
}

static bool tomoyo_is_globally_readable_file(const struct tomoyo_path_info *
					     filename)
{
	struct tomoyo_globally_readable_file_entry *ptr;
	bool found = false;

	list_for_each_entry_rcu(ptr, &tomoyo_globally_readable_list, list) {
		if (!ptr->is_deleted &&
		    tomoyo_path_matches_pattern(filename, ptr->filename)) {
			found = true;
			break;
		}
	}
	return found;
}

int tomoyo_write_globally_readable_policy(char *data, const bool is_delete)
{
	return tomoyo_update_globally_readable_entry(data, is_delete);
}

bool tomoyo_read_globally_readable_policy(struct tomoyo_io_buffer *head)
{
	struct list_head *pos;
	bool done = true;

	list_for_each_cookie(pos, head->read_var2,
			     &tomoyo_globally_readable_list) {
		struct tomoyo_globally_readable_file_entry *ptr;
		ptr = list_entry(pos,
				 struct tomoyo_globally_readable_file_entry,
				 list);
		if (ptr->is_deleted)
			continue;
		done = tomoyo_io_printf(head, TOMOYO_KEYWORD_ALLOW_READ "%s\n",
					ptr->filename->name);
		if (!done)
			break;
	}
	return done;
}

LIST_HEAD(tomoyo_pattern_list);

static int tomoyo_update_file_pattern_entry(const char *pattern,
					    const bool is_delete)
{
	struct tomoyo_pattern_entry *ptr;
	struct tomoyo_pattern_entry e = { .pattern = tomoyo_get_name(pattern) };
	int error = is_delete ? -ENOENT : -ENOMEM;

	if (!e.pattern)
		return error;
	if (!e.pattern->is_patterned)
		goto out;
	if (mutex_lock_interruptible(&tomoyo_policy_lock))
		goto out;
	list_for_each_entry_rcu(ptr, &tomoyo_pattern_list, list) {
		if (e.pattern != ptr->pattern)
			continue;
		ptr->is_deleted = is_delete;
		error = 0;
		break;
	}
	if (!is_delete && error) {
		struct tomoyo_pattern_entry *entry =
			tomoyo_commit_ok(&e, sizeof(e));
		if (entry) {
			list_add_tail_rcu(&entry->list, &tomoyo_pattern_list);
			error = 0;
		}
	}
	mutex_unlock(&tomoyo_policy_lock);
 out:
	tomoyo_put_name(e.pattern);
	return error;
}

static const struct tomoyo_path_info *
tomoyo_get_file_pattern(const struct tomoyo_path_info *filename)
{
	struct tomoyo_pattern_entry *ptr;
	const struct tomoyo_path_info *pattern = NULL;

	list_for_each_entry_rcu(ptr, &tomoyo_pattern_list, list) {
		if (ptr->is_deleted)
			continue;
		if (!tomoyo_path_matches_pattern(filename, ptr->pattern))
			continue;
		pattern = ptr->pattern;
		if (tomoyo_strendswith(pattern->name, "/\\*")) {
			/* Do nothing. Try to find the better match. */
		} else {
			/* This would be the better match. Use this. */
			break;
		}
	}
	if (pattern)
		filename = pattern;
	return filename;
}

int tomoyo_write_pattern_policy(char *data, const bool is_delete)
{
	return tomoyo_update_file_pattern_entry(data, is_delete);
}

bool tomoyo_read_file_pattern(struct tomoyo_io_buffer *head)
{
	struct list_head *pos;
	bool done = true;

	list_for_each_cookie(pos, head->read_var2, &tomoyo_pattern_list) {
		struct tomoyo_pattern_entry *ptr;
		ptr = list_entry(pos, struct tomoyo_pattern_entry, list);
		if (ptr->is_deleted)
			continue;
		done = tomoyo_io_printf(head, TOMOYO_KEYWORD_FILE_PATTERN
					"%s\n", ptr->pattern->name);
		if (!done)
			break;
	}
	return done;
}

LIST_HEAD(tomoyo_no_rewrite_list);

static int tomoyo_update_no_rewrite_entry(const char *pattern,
					  const bool is_delete)
{
	struct tomoyo_no_rewrite_entry *ptr;
	struct tomoyo_no_rewrite_entry e = { };
	int error = is_delete ? -ENOENT : -ENOMEM;

	if (!tomoyo_is_correct_path(pattern, 0, 0, 0))
		return -EINVAL;
	e.pattern = tomoyo_get_name(pattern);
	if (!e.pattern)
		return error;
	if (mutex_lock_interruptible(&tomoyo_policy_lock))
		goto out;
	list_for_each_entry_rcu(ptr, &tomoyo_no_rewrite_list, list) {
		if (ptr->pattern != e.pattern)
			continue;
		ptr->is_deleted = is_delete;
		error = 0;
		break;
	}
	if (!is_delete && error) {
		struct tomoyo_no_rewrite_entry *entry =
			tomoyo_commit_ok(&e, sizeof(e));
		if (entry) {
			list_add_tail_rcu(&entry->list,
					  &tomoyo_no_rewrite_list);
			error = 0;
		}
	}
	mutex_unlock(&tomoyo_policy_lock);
 out:
	tomoyo_put_name(e.pattern);
	return error;
}

static bool tomoyo_is_no_rewrite_file(const struct tomoyo_path_info *filename)
{
	struct tomoyo_no_rewrite_entry *ptr;
	bool found = false;

	list_for_each_entry_rcu(ptr, &tomoyo_no_rewrite_list, list) {
		if (ptr->is_deleted)
			continue;
		if (!tomoyo_path_matches_pattern(filename, ptr->pattern))
			continue;
		found = true;
		break;
	}
	return found;
}

int tomoyo_write_no_rewrite_policy(char *data, const bool is_delete)
{
	return tomoyo_update_no_rewrite_entry(data, is_delete);
}

bool tomoyo_read_no_rewrite_policy(struct tomoyo_io_buffer *head)
{
	struct list_head *pos;
	bool done = true;

	list_for_each_cookie(pos, head->read_var2, &tomoyo_no_rewrite_list) {
		struct tomoyo_no_rewrite_entry *ptr;
		ptr = list_entry(pos, struct tomoyo_no_rewrite_entry, list);
		if (ptr->is_deleted)
			continue;
		done = tomoyo_io_printf(head, TOMOYO_KEYWORD_DENY_REWRITE
					"%s\n", ptr->pattern->name);
		if (!done)
			break;
	}
	return done;
}

static int tomoyo_update_file_acl(const char *filename, u8 perm,
				  struct tomoyo_domain_info * const domain,
				  const bool is_delete)
{
	if (perm > 7 || !perm) {
		printk(KERN_DEBUG "%s: Invalid permission '%d %s'\n",
		       __func__, perm, filename);
		return -EINVAL;
	}
	if (filename[0] != '@' && tomoyo_strendswith(filename, "/"))
		/*
		 * Only 'allow_mkdir' and 'allow_rmdir' are valid for
		 * directory permissions.
		 */
		return 0;
	if (perm & 4)
		tomoyo_update_path_acl(TOMOYO_TYPE_READ, filename, domain,
				       is_delete);
	if (perm & 2)
		tomoyo_update_path_acl(TOMOYO_TYPE_WRITE, filename, domain,
				       is_delete);
	if (perm & 1)
		tomoyo_update_path_acl(TOMOYO_TYPE_EXECUTE, filename, domain,
				       is_delete);
	return 0;
}

static int tomoyo_path_acl2(const struct tomoyo_domain_info *domain,
			    const struct tomoyo_path_info *filename,
			    const u32 perm, const bool may_use_pattern)
{
	struct tomoyo_acl_info *ptr;
	int error = -EPERM;

	list_for_each_entry_rcu(ptr, &domain->acl_info_list, list) {
		struct tomoyo_path_acl *acl;
		if (ptr->type != TOMOYO_TYPE_PATH_ACL)
			continue;
		acl = container_of(ptr, struct tomoyo_path_acl, head);
		if (perm <= 0xFFFF) {
			if (!(acl->perm & perm))
				continue;
		} else {
			if (!(acl->perm_high & (perm >> 16)))
				continue;
		}
		if (!tomoyo_compare_name_union_pattern(filename, &acl->name,
                                                       may_use_pattern))
			continue;
		error = 0;
		break;
	}
	return error;
}

static int tomoyo_check_file_acl(const struct tomoyo_domain_info *domain,
				 const struct tomoyo_path_info *filename,
				 const u8 operation)
{
	u32 perm = 0;

	if (!tomoyo_check_flags(domain, TOMOYO_MAC_FOR_FILE))
		return 0;
	if (operation == 6)
		perm = 1 << TOMOYO_TYPE_READ_WRITE;
	else if (operation == 4)
		perm = 1 << TOMOYO_TYPE_READ;
	else if (operation == 2)
		perm = 1 << TOMOYO_TYPE_WRITE;
	else if (operation == 1)
		perm = 1 << TOMOYO_TYPE_EXECUTE;
	else
		BUG();
	return tomoyo_path_acl2(domain, filename, perm, operation != 1);
}

static int tomoyo_check_file_perm2(struct tomoyo_domain_info * const domain,
				   const struct tomoyo_path_info *filename,
				   const u8 perm, const char *operation,
				   const u8 mode)
{
	const bool is_enforce = (mode == 3);
	const char *msg = "<unknown>";
	int error = 0;

	if (!filename)
		return 0;
	error = tomoyo_check_file_acl(domain, filename, perm);
	if (error && perm == 4 && !domain->ignore_global_allow_read
	    && tomoyo_is_globally_readable_file(filename))
		error = 0;
	if (perm == 6)
		msg = tomoyo_path2keyword(TOMOYO_TYPE_READ_WRITE);
	else if (perm == 4)
		msg = tomoyo_path2keyword(TOMOYO_TYPE_READ);
	else if (perm == 2)
		msg = tomoyo_path2keyword(TOMOYO_TYPE_WRITE);
	else if (perm == 1)
		msg = tomoyo_path2keyword(TOMOYO_TYPE_EXECUTE);
	else
		BUG();
	if (!error)
		return 0;
	if (tomoyo_verbose_mode(domain))
		printk(KERN_WARNING "TOMOYO-%s: Access '%s(%s) %s' denied "
		       "for %s\n", tomoyo_get_msg(is_enforce), msg, operation,
		       filename->name, tomoyo_get_last_name(domain));
	if (is_enforce)
		return error;
	if (mode == 1 && tomoyo_domain_quota_is_ok(domain)) {
		/* Don't use patterns for execute permission. */
		const struct tomoyo_path_info *patterned_file = (perm != 1) ?
			tomoyo_get_file_pattern(filename) : filename;
		tomoyo_update_file_acl(patterned_file->name, perm,
				       domain, false);
	}
	return 0;
}

int tomoyo_write_file_policy(char *data, struct tomoyo_domain_info *domain,
			     const bool is_delete)
{
	char *filename = strchr(data, ' ');
	char *filename2;
	unsigned int perm;
	u8 type;

	if (!filename)
		return -EINVAL;
	*filename++ = '\0';
	if (sscanf(data, "%u", &perm) == 1)
		return tomoyo_update_file_acl(filename, (u8) perm, domain,
					      is_delete);
	if (strncmp(data, "allow_", 6))
		goto out;
	data += 6;
	for (type = 0; type < TOMOYO_MAX_PATH_OPERATION; type++) {
		if (strcmp(data, tomoyo_path_keyword[type]))
			continue;
		return tomoyo_update_path_acl(type, filename, domain,
					      is_delete);
	}
	filename2 = strchr(filename, ' ');
	if (!filename2)
		goto out;
	*filename2++ = '\0';
	for (type = 0; type < TOMOYO_MAX_PATH2_OPERATION; type++) {
		if (strcmp(data, tomoyo_path2_keyword[type]))
			continue;
		return tomoyo_update_path2_acl(type, filename, filename2,
					       domain, is_delete);
	}
 out:
	return -EINVAL;
}

static int tomoyo_update_path_acl(const u8 type, const char *filename,
				  struct tomoyo_domain_info *const domain,
				  const bool is_delete)
{
	static const u32 tomoyo_rw_mask =
		(1 << TOMOYO_TYPE_READ) | (1 << TOMOYO_TYPE_WRITE);
	const u32 perm = 1 << type;
	struct tomoyo_acl_info *ptr;
	struct tomoyo_path_acl e = {
		.head.type = TOMOYO_TYPE_PATH_ACL,
		.perm_high = perm >> 16,
		.perm = perm
	};
	int error = is_delete ? -ENOENT : -ENOMEM;

	if (type == TOMOYO_TYPE_READ_WRITE)
		e.perm |= tomoyo_rw_mask;
	if (!domain)
		return -EINVAL;
	if (!tomoyo_parse_name_union(filename, &e.name))
		return -EINVAL;
	if (mutex_lock_interruptible(&tomoyo_policy_lock))
		goto out;
	list_for_each_entry_rcu(ptr, &domain->acl_info_list, list) {
		struct tomoyo_path_acl *acl =
			container_of(ptr, struct tomoyo_path_acl, head);
		if (!tomoyo_is_same_path_acl(acl, &e))
			continue;
		if (is_delete) {
			if (perm <= 0xFFFF)
				acl->perm &= ~perm;
			else
				acl->perm_high &= ~(perm >> 16);
			if ((acl->perm & tomoyo_rw_mask) != tomoyo_rw_mask)
				acl->perm &= ~(1 << TOMOYO_TYPE_READ_WRITE);
			else if (!(acl->perm & (1 << TOMOYO_TYPE_READ_WRITE)))
				acl->perm &= ~tomoyo_rw_mask;
		} else {
			if (perm <= 0xFFFF)
				acl->perm |= perm;
			else
				acl->perm_high |= (perm >> 16);
			if ((acl->perm & tomoyo_rw_mask) == tomoyo_rw_mask)
				acl->perm |= 1 << TOMOYO_TYPE_READ_WRITE;
			else if (acl->perm & (1 << TOMOYO_TYPE_READ_WRITE))
				acl->perm |= tomoyo_rw_mask;
		}
		error = 0;
		break;
	}
	if (!is_delete && error) {
		struct tomoyo_path_acl *entry =
			tomoyo_commit_ok(&e, sizeof(e));
		if (entry) {
			list_add_tail_rcu(&entry->head.list,
					  &domain->acl_info_list);
			error = 0;
		}
	}
	mutex_unlock(&tomoyo_policy_lock);
 out:
	tomoyo_put_name_union(&e.name);
	return error;
}

static int tomoyo_update_path2_acl(const u8 type, const char *filename1,
				   const char *filename2,
				   struct tomoyo_domain_info *const domain,
				   const bool is_delete)
{
	const u8 perm = 1 << type;
	struct tomoyo_path2_acl e = {
		.head.type = TOMOYO_TYPE_PATH2_ACL,
		.perm = perm
	};
	struct tomoyo_acl_info *ptr;
	int error = is_delete ? -ENOENT : -ENOMEM;

	if (!domain)
		return -EINVAL;
	if (!tomoyo_parse_name_union(filename1, &e.name1) ||
	    !tomoyo_parse_name_union(filename2, &e.name2))
		goto out;
	if (mutex_lock_interruptible(&tomoyo_policy_lock))
		goto out;
	list_for_each_entry_rcu(ptr, &domain->acl_info_list, list) {
		struct tomoyo_path2_acl *acl =
			container_of(ptr, struct tomoyo_path2_acl, head);
		if (!tomoyo_is_same_path2_acl(acl, &e))
			continue;
		if (is_delete)
			acl->perm &= ~perm;
		else
			acl->perm |= perm;
		error = 0;
		break;
	}
	if (!is_delete && error) {
		struct tomoyo_path2_acl *entry =
			tomoyo_commit_ok(&e, sizeof(e));
		if (entry) {
			list_add_tail_rcu(&entry->head.list,
					  &domain->acl_info_list);
			error = 0;
		}
	}
	mutex_unlock(&tomoyo_policy_lock);
 out:
	tomoyo_put_name_union(&e.name1);
	tomoyo_put_name_union(&e.name2);
	return error;
}

static int tomoyo_path_acl(struct tomoyo_domain_info *domain, const u8 type,
			   const struct tomoyo_path_info *filename)
{
	if (!tomoyo_check_flags(domain, TOMOYO_MAC_FOR_FILE))
		return 0;
	return tomoyo_path_acl2(domain, filename, 1 << type, 1);
}

static int tomoyo_path2_acl(const struct tomoyo_domain_info *domain,
			    const u8 type,
			    const struct tomoyo_path_info *filename1,
			    const struct tomoyo_path_info *filename2)
{
	struct tomoyo_acl_info *ptr;
	const u8 perm = 1 << type;
	int error = -EPERM;

	if (!tomoyo_check_flags(domain, TOMOYO_MAC_FOR_FILE))
		return 0;
	list_for_each_entry_rcu(ptr, &domain->acl_info_list, list) {
		struct tomoyo_path2_acl *acl;
		if (ptr->type != TOMOYO_TYPE_PATH2_ACL)
			continue;
		acl = container_of(ptr, struct tomoyo_path2_acl, head);
		if (!(acl->perm & perm))
			continue;
		if (!tomoyo_compare_name_union(filename1, &acl->name1))
			continue;
		if (!tomoyo_compare_name_union(filename2, &acl->name2))
			continue;
		error = 0;
		break;
	}
	return error;
}

static int tomoyo_path_permission2(struct tomoyo_domain_info *const domain,
				   u8 operation,
				   const struct tomoyo_path_info *filename,
				   const u8 mode)
{
	const char *msg;
	int error;
	const bool is_enforce = (mode == 3);

	if (!mode)
		return 0;
 next:
	error = tomoyo_path_acl(domain, operation, filename);
	msg = tomoyo_path2keyword(operation);
	if (!error)
		goto ok;
	if (tomoyo_verbose_mode(domain))
		printk(KERN_WARNING "TOMOYO-%s: Access '%s %s' denied for %s\n",
		       tomoyo_get_msg(is_enforce), msg, filename->name,
		       tomoyo_get_last_name(domain));
	if (mode == 1 && tomoyo_domain_quota_is_ok(domain)) {
		const char *name = tomoyo_get_file_pattern(filename)->name;
		tomoyo_update_path_acl(operation, name, domain, false);
	}
	if (!is_enforce)
		error = 0;
 ok:
	/*
	 * Since "allow_truncate" doesn't imply "allow_rewrite" permission,
	 * we need to check "allow_rewrite" permission if the filename is
	 * specified by "deny_rewrite" keyword.
	 */
	if (!error && operation == TOMOYO_TYPE_TRUNCATE &&
	    tomoyo_is_no_rewrite_file(filename)) {
		operation = TOMOYO_TYPE_REWRITE;
		goto next;
	}
	return error;
}

int tomoyo_check_exec_perm(struct tomoyo_domain_info *domain,
			   const struct tomoyo_path_info *filename)
{
	const u8 mode = tomoyo_check_flags(domain, TOMOYO_MAC_FOR_FILE);

	if (!mode)
		return 0;
	return tomoyo_check_file_perm2(domain, filename, 1, "do_execve", mode);
}

int tomoyo_check_open_permission(struct tomoyo_domain_info *domain,
				 struct path *path, const int flag)
{
	const u8 acc_mode = ACC_MODE(flag);
	int error = -ENOMEM;
	struct tomoyo_path_info *buf;
	const u8 mode = tomoyo_check_flags(domain, TOMOYO_MAC_FOR_FILE);
	const bool is_enforce = (mode == 3);
	int idx;

	if (!mode || !path->mnt)
		return 0;
	if (acc_mode == 0)
		return 0;
	if (path->dentry->d_inode && S_ISDIR(path->dentry->d_inode->i_mode))
		/*
		 * I don't check directories here because mkdir() and rmdir()
		 * don't call me.
		 */
		return 0;
	idx = tomoyo_read_lock();
	buf = tomoyo_get_path(path);
	if (!buf)
		goto out;
	error = 0;
	/*
	 * If the filename is specified by "deny_rewrite" keyword,
	 * we need to check "allow_rewrite" permission when the filename is not
	 * opened for append mode or the filename is truncated at open time.
	 */
	if ((acc_mode & MAY_WRITE) &&
	    ((flag & O_TRUNC) || !(flag & O_APPEND)) &&
	    (tomoyo_is_no_rewrite_file(buf))) {
		error = tomoyo_path_permission2(domain, TOMOYO_TYPE_REWRITE,
						buf, mode);
	}
	if (!error)
		error = tomoyo_check_file_perm2(domain, buf, acc_mode, "open",
						mode);
	if (!error && (flag & O_TRUNC))
		error = tomoyo_path_permission2(domain, TOMOYO_TYPE_TRUNCATE,
						buf, mode);
 out:
	kfree(buf);
	tomoyo_read_unlock(idx);
	if (!is_enforce)
		error = 0;
	return error;
}

int tomoyo_path_perm(const u8 operation, struct path *path)
{
	int error = -ENOMEM;
	struct tomoyo_path_info *buf;
	struct tomoyo_domain_info *domain = tomoyo_domain();
	const u8 mode = tomoyo_check_flags(domain, TOMOYO_MAC_FOR_FILE);
	const bool is_enforce = (mode == 3);
	int idx;

	if (!mode || !path->mnt)
		return 0;
	idx = tomoyo_read_lock();
	buf = tomoyo_get_path(path);
	if (!buf)
		goto out;
	switch (operation) {
	case TOMOYO_TYPE_MKDIR:
	case TOMOYO_TYPE_RMDIR:
	case TOMOYO_TYPE_CHROOT:
		if (!buf->is_dir) {
			/*
			 * tomoyo_get_path() reserves space for appending "/."
			 */
			strcat((char *) buf->name, "/");
			tomoyo_fill_path_info(buf);
		}
	}
	error = tomoyo_path_permission2(domain, operation, buf, mode);
 out:
	kfree(buf);
	tomoyo_read_unlock(idx);
	if (!is_enforce)
		error = 0;
	return error;
}

int tomoyo_check_rewrite_permission(struct file *filp)
{
	int error = -ENOMEM;
	struct tomoyo_domain_info *domain = tomoyo_domain();
	const u8 mode = tomoyo_check_flags(domain, TOMOYO_MAC_FOR_FILE);
	const bool is_enforce = (mode == 3);
	struct tomoyo_path_info *buf;
	int idx;

	if (!mode || !filp->f_path.mnt)
		return 0;

	idx = tomoyo_read_lock();
	buf = tomoyo_get_path(&filp->f_path);
	if (!buf)
		goto out;
	if (!tomoyo_is_no_rewrite_file(buf)) {
		error = 0;
		goto out;
	}
	error = tomoyo_path_permission2(domain, TOMOYO_TYPE_REWRITE, buf, mode);
 out:
	kfree(buf);
	tomoyo_read_unlock(idx);
	if (!is_enforce)
		error = 0;
	return error;
}

int tomoyo_path2_perm(const u8 operation, struct path *path1,
		      struct path *path2)
{
	int error = -ENOMEM;
	struct tomoyo_path_info *buf1, *buf2;
	struct tomoyo_domain_info *domain = tomoyo_domain();
	const u8 mode = tomoyo_check_flags(domain, TOMOYO_MAC_FOR_FILE);
	const bool is_enforce = (mode == 3);
	const char *msg;
	int idx;

	if (!mode || !path1->mnt || !path2->mnt)
		return 0;
	idx = tomoyo_read_lock();
	buf1 = tomoyo_get_path(path1);
	buf2 = tomoyo_get_path(path2);
	if (!buf1 || !buf2)
		goto out;
	{
		struct dentry *dentry = path1->dentry;
		if (dentry->d_inode && S_ISDIR(dentry->d_inode->i_mode)) {
			/*
			 * tomoyo_get_path() reserves space for appending "/."
			 */
			if (!buf1->is_dir) {
				strcat((char *) buf1->name, "/");
				tomoyo_fill_path_info(buf1);
			}
			if (!buf2->is_dir) {
				strcat((char *) buf2->name, "/");
				tomoyo_fill_path_info(buf2);
			}
		}
	}
	error = tomoyo_path2_acl(domain, operation, buf1, buf2);
	msg = tomoyo_path22keyword(operation);
	if (!error)
		goto out;
	if (tomoyo_verbose_mode(domain))
		printk(KERN_WARNING "TOMOYO-%s: Access '%s %s %s' "
		       "denied for %s\n", tomoyo_get_msg(is_enforce),
		       msg, buf1->name, buf2->name,
		       tomoyo_get_last_name(domain));
	if (mode == 1 && tomoyo_domain_quota_is_ok(domain)) {
		const char *name1 = tomoyo_get_file_pattern(buf1)->name;
		const char *name2 = tomoyo_get_file_pattern(buf2)->name;
		tomoyo_update_path2_acl(operation, name1, name2, domain,
					false);
	}
 out:
	kfree(buf1);
	kfree(buf2);
	tomoyo_read_unlock(idx);
	if (!is_enforce)
		error = 0;
	return error;
}
