
#include <sys/types.h>
#ifdef _EVENT_HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define EVENT_NO_STRUCT
#include <event.h>

#ifdef _EVENT___func__
#define __func__ _EVENT___func__
#endif

#include "./regress.gen.h"

void event_err(int eval, const char *fmt, ...);
void event_warn(const char *fmt, ...);
void event_errx(int eval, const char *fmt, ...);
void event_warnx(const char *fmt, ...);



static struct msg_access_ __msg_base = {
  msg_from_name_assign,
  msg_from_name_get,
  msg_to_name_assign,
  msg_to_name_get,
  msg_attack_assign,
  msg_attack_get,
  msg_run_assign,
  msg_run_get,
  msg_run_add,
};

struct msg *
msg_new(void)
{
  struct msg *tmp;
  if ((tmp = malloc(sizeof(struct msg))) == NULL) {
    event_warn("%s: malloc", __func__);
    return (NULL);
  }
  tmp->base = &__msg_base;

  tmp->from_name_data = NULL;
  tmp->from_name_set = 0;

  tmp->to_name_data = NULL;
  tmp->to_name_set = 0;

  tmp->attack_data = NULL;
  tmp->attack_set = 0;

  tmp->run_data = NULL;
  tmp->run_length = 0;
  tmp->run_num_allocated = 0;
  tmp->run_set = 0;

  return (tmp);
}




struct run *
msg_run_add(struct msg *msg)
{
  if (++msg->run_length >= msg->run_num_allocated) {
    int tobe_allocated = msg->run_num_allocated;
    struct run ** new_data = NULL;
    tobe_allocated = !tobe_allocated ? 1 : tobe_allocated << 1;
    new_data = (struct run **) realloc(msg->run_data,
        tobe_allocated * sizeof(struct run *));
    if (new_data == NULL)
      goto error;
    msg->run_data = new_data;
    msg->run_num_allocated = tobe_allocated;
  }
  msg->run_data[msg->run_length - 1] = run_new();
  if (msg->run_data[msg->run_length - 1] == NULL)
    goto error;
  msg->run_set = 1;
  return (msg->run_data[msg->run_length - 1]);
error:
  --msg->run_length;
  return (NULL);
}
        

int
msg_from_name_assign(struct msg *msg,
    const char * value)
{
  if (msg->from_name_data != NULL)
    free(msg->from_name_data);
  if ((msg->from_name_data = strdup(value)) == NULL)
    return (-1);
  msg->from_name_set = 1;
  return (0);
}

int
msg_to_name_assign(struct msg *msg,
    const char * value)
{
  if (msg->to_name_data != NULL)
    free(msg->to_name_data);
  if ((msg->to_name_data = strdup(value)) == NULL)
    return (-1);
  msg->to_name_set = 1;
  return (0);
}

int
msg_attack_assign(struct msg *msg,
    const struct kill* value)
{
   struct evbuffer *tmp = NULL;
   if (msg->attack_set) {
     kill_clear(msg->attack_data);
     msg->attack_set = 0;
   } else {
     msg->attack_data = kill_new();
     if (msg->attack_data == NULL) {
       event_warn("%s: kill_new()", __func__);
       goto error;
     }
   }
   if ((tmp = evbuffer_new()) == NULL) {
     event_warn("%s: evbuffer_new()", __func__);
     goto error;
   }
   kill_marshal(tmp, value);
   if (kill_unmarshal(msg->attack_data, tmp) == -1) {
     event_warnx("%s: kill_unmarshal", __func__);
     goto error;
   }
   msg->attack_set = 1;
   evbuffer_free(tmp);
   return (0);
 error:
   if (tmp != NULL)
     evbuffer_free(tmp);
   if (msg->attack_data != NULL) {
     kill_free(msg->attack_data);
     msg->attack_data = NULL;
   }
   return (-1);
}

int
msg_run_assign(struct msg *msg, int off,
    const struct run * value)
{
  struct evbuffer *tmp = NULL;
  if (!msg->run_set || off < 0 || off >= msg->run_length)
    return (-1);
  run_clear(msg->run_data[off]);
  if ((tmp = evbuffer_new()) == NULL) {
    event_warn("%s: evbuffer_new()", __func__);
    goto error;
  }
  run_marshal(tmp, value);
  if (run_unmarshal(msg->run_data[off], tmp) == -1) {
    event_warnx("%s: run_unmarshal", __func__);
    goto error;
  }
  evbuffer_free(tmp);
  return (0);
error:
  if (tmp != NULL)
    evbuffer_free(tmp);
  run_clear(msg->run_data[off]);
  return (-1);
}

int
msg_from_name_get(struct msg *msg, char * *value)
{
  if (msg->from_name_set != 1)
    return (-1);
  *value = msg->from_name_data;
  return (0);
}

int
msg_to_name_get(struct msg *msg, char * *value)
{
  if (msg->to_name_set != 1)
    return (-1);
  *value = msg->to_name_data;
  return (0);
}

int
msg_attack_get(struct msg *msg, struct kill* *value)
{
  if (msg->attack_set != 1) {
    msg->attack_data = kill_new();
    if (msg->attack_data == NULL)
      return (-1);
    msg->attack_set = 1;
  }
  *value = msg->attack_data;
  return (0);
}

int
msg_run_get(struct msg *msg, int offset,
    struct run * *value)
{
  if (!msg->run_set || offset < 0 || offset >= msg->run_length)
    return (-1);
  *value = msg->run_data[offset];
  return (0);
}

void
msg_clear(struct msg *tmp)
{
  if (tmp->from_name_set == 1) {
    free (tmp->from_name_data);
    tmp->from_name_data = NULL;
    tmp->from_name_set = 0;
  }
  if (tmp->to_name_set == 1) {
    free (tmp->to_name_data);
    tmp->to_name_data = NULL;
    tmp->to_name_set = 0;
  }
  if (tmp->attack_set == 1) {
    kill_free(tmp->attack_data);
    tmp->attack_data = NULL;
    tmp->attack_set = 0;
  }
  if (tmp->run_set == 1) {
    int i;
    for (i = 0; i < tmp->run_length; ++i) {
      run_free(tmp->run_data[i]);
    }
    free(tmp->run_data);
    tmp->run_data = NULL;
    tmp->run_set = 0;
    tmp->run_length = 0;
    tmp->run_num_allocated = 0;
  }
}

void
msg_free(struct msg *tmp)
{
  if (tmp->from_name_data != NULL)
      free (tmp->from_name_data); 
  if (tmp->to_name_data != NULL)
      free (tmp->to_name_data); 
  if (tmp->attack_data != NULL)
      kill_free(tmp->attack_data); 
  if (tmp->run_data != NULL) {
    int i;
    for (i = 0; i < tmp->run_length; ++i) {
      run_free(tmp->run_data[i]); 
      tmp->run_data[i] = NULL;
    }
    free(tmp->run_data);
    tmp->run_data = NULL;
    tmp->run_length = 0;
    tmp->run_num_allocated = 0;
  }
  free(tmp);
}

void
msg_marshal(struct evbuffer *evbuf, const struct msg *tmp){
  evtag_marshal_string(evbuf, MSG_FROM_NAME, tmp->from_name_data);
  evtag_marshal_string(evbuf, MSG_TO_NAME, tmp->to_name_data);
  if (tmp->attack_set) {
    evtag_marshal_kill(evbuf, MSG_ATTACK, tmp->attack_data);
  }
  {
    int i;
    for (i = 0; i < tmp->run_length; ++i) {
      evtag_marshal_run(evbuf, MSG_RUN, tmp->run_data[i]);
    }
  }
}

int
msg_unmarshal(struct msg *tmp,  struct evbuffer *evbuf)
{
  ev_uint32_t tag;
  while (EVBUFFER_LENGTH(evbuf) > 0) {
    if (evtag_peek(evbuf, &tag) == -1)
      return (-1);
    switch (tag) {

      case MSG_FROM_NAME:

        if (tmp->from_name_set)
          return (-1);
        if (evtag_unmarshal_string(evbuf, MSG_FROM_NAME, &tmp->from_name_data) == -1) {
          event_warnx("%s: failed to unmarshal from_name", __func__);
          return (-1);
        }
        tmp->from_name_set = 1;
        break;

      case MSG_TO_NAME:

        if (tmp->to_name_set)
          return (-1);
        if (evtag_unmarshal_string(evbuf, MSG_TO_NAME, &tmp->to_name_data) == -1) {
          event_warnx("%s: failed to unmarshal to_name", __func__);
          return (-1);
        }
        tmp->to_name_set = 1;
        break;

      case MSG_ATTACK:

        if (tmp->attack_set)
          return (-1);
        tmp->attack_data = kill_new();
        if (tmp->attack_data == NULL)
          return (-1);
        if (evtag_unmarshal_kill(evbuf, MSG_ATTACK, tmp->attack_data) == -1) {
          event_warnx("%s: failed to unmarshal attack", __func__);
          return (-1);
        }
        tmp->attack_set = 1;
        break;

      case MSG_RUN:

        if (msg_run_add(tmp) == NULL)
          return (-1);
        if (evtag_unmarshal_run(evbuf, MSG_RUN,
          tmp->run_data[tmp->run_length - 1]) == -1) {
          --tmp->run_length;
          event_warnx("%s: failed to unmarshal run", __func__);
          return (-1);
        }
        tmp->run_set = 1;
        break;

      default:
        return -1;
    }
  }

  if (msg_complete(tmp) == -1)
    return (-1);
  return (0);
}

int
msg_complete(struct msg *msg)
{
  if (!msg->from_name_set)
    return (-1);
  if (!msg->to_name_set)
    return (-1);
  if (msg->attack_set && kill_complete(msg->attack_data) == -1)
    return (-1);
  {
    int i;
    for (i = 0; i < msg->run_length; ++i) {
      if (run_complete(msg->run_data[i]) == -1)
        return (-1);
    }
  }
  return (0);
}

int
evtag_unmarshal_msg(struct evbuffer *evbuf, ev_uint32_t need_tag, struct msg *msg)
{
  ev_uint32_t tag;
  int res = -1;

  struct evbuffer *tmp = evbuffer_new();

  if (evtag_unmarshal(evbuf, &tag, tmp) == -1 || tag != need_tag)
    goto error;

  if (msg_unmarshal(msg, tmp) == -1)
    goto error;

  res = 0;

 error:
  evbuffer_free(tmp);
  return (res);
}

void
evtag_marshal_msg(struct evbuffer *evbuf, ev_uint32_t tag, const struct msg *msg)
{
  struct evbuffer *_buf = evbuffer_new();
  assert(_buf != NULL);
  evbuffer_drain(_buf, -1);
  msg_marshal(_buf, msg);
  evtag_marshal(evbuf, tag, EVBUFFER_DATA(_buf), EVBUFFER_LENGTH(_buf));
  evbuffer_free(_buf);
}


static struct kill_access_ __kill_base = {
  kill_weapon_assign,
  kill_weapon_get,
  kill_action_assign,
  kill_action_get,
  kill_how_often_assign,
  kill_how_often_get,
};

struct kill *
kill_new(void)
{
  struct kill *tmp;
  if ((tmp = malloc(sizeof(struct kill))) == NULL) {
    event_warn("%s: malloc", __func__);
    return (NULL);
  }
  tmp->base = &__kill_base;

  tmp->weapon_data = NULL;
  tmp->weapon_set = 0;

  tmp->action_data = NULL;
  tmp->action_set = 0;

  tmp->how_often_data = 0;
  tmp->how_often_set = 0;

  return (tmp);
}




int
kill_weapon_assign(struct kill *msg,
    const char * value)
{
  if (msg->weapon_data != NULL)
    free(msg->weapon_data);
  if ((msg->weapon_data = strdup(value)) == NULL)
    return (-1);
  msg->weapon_set = 1;
  return (0);
}

int
kill_action_assign(struct kill *msg,
    const char * value)
{
  if (msg->action_data != NULL)
    free(msg->action_data);
  if ((msg->action_data = strdup(value)) == NULL)
    return (-1);
  msg->action_set = 1;
  return (0);
}

int
kill_how_often_assign(struct kill *msg, const ev_uint32_t value)
{
  msg->how_often_set = 1;
  msg->how_often_data = value;
  return (0);
}

int
kill_weapon_get(struct kill *msg, char * *value)
{
  if (msg->weapon_set != 1)
    return (-1);
  *value = msg->weapon_data;
  return (0);
}

int
kill_action_get(struct kill *msg, char * *value)
{
  if (msg->action_set != 1)
    return (-1);
  *value = msg->action_data;
  return (0);
}

int
kill_how_often_get(struct kill *msg, ev_uint32_t *value)
{
  if (msg->how_often_set != 1)
    return (-1);
  *value = msg->how_often_data;
  return (0);
}

void
kill_clear(struct kill *tmp)
{
  if (tmp->weapon_set == 1) {
    free (tmp->weapon_data);
    tmp->weapon_data = NULL;
    tmp->weapon_set = 0;
  }
  if (tmp->action_set == 1) {
    free (tmp->action_data);
    tmp->action_data = NULL;
    tmp->action_set = 0;
  }
  tmp->how_often_set = 0;
}

void
kill_free(struct kill *tmp)
{
  if (tmp->weapon_data != NULL)
      free (tmp->weapon_data); 
  if (tmp->action_data != NULL)
      free (tmp->action_data); 
  free(tmp);
}

void
kill_marshal(struct evbuffer *evbuf, const struct kill *tmp){
  evtag_marshal_string(evbuf, KILL_WEAPON, tmp->weapon_data);
  evtag_marshal_string(evbuf, KILL_ACTION, tmp->action_data);
  if (tmp->how_often_set) {
    evtag_marshal_int(evbuf, KILL_HOW_OFTEN, tmp->how_often_data);
  }
}

int
kill_unmarshal(struct kill *tmp,  struct evbuffer *evbuf)
{
  ev_uint32_t tag;
  while (EVBUFFER_LENGTH(evbuf) > 0) {
    if (evtag_peek(evbuf, &tag) == -1)
      return (-1);
    switch (tag) {

      case KILL_WEAPON:

        if (tmp->weapon_set)
          return (-1);
        if (evtag_unmarshal_string(evbuf, KILL_WEAPON, &tmp->weapon_data) == -1) {
          event_warnx("%s: failed to unmarshal weapon", __func__);
          return (-1);
        }
        tmp->weapon_set = 1;
        break;

      case KILL_ACTION:

        if (tmp->action_set)
          return (-1);
        if (evtag_unmarshal_string(evbuf, KILL_ACTION, &tmp->action_data) == -1) {
          event_warnx("%s: failed to unmarshal action", __func__);
          return (-1);
        }
        tmp->action_set = 1;
        break;

      case KILL_HOW_OFTEN:

        if (tmp->how_often_set)
          return (-1);
        if (evtag_unmarshal_int(evbuf, KILL_HOW_OFTEN, &tmp->how_often_data) == -1) {
          event_warnx("%s: failed to unmarshal how_often", __func__);
          return (-1);
        }
        tmp->how_often_set = 1;
        break;

      default:
        return -1;
    }
  }

  if (kill_complete(tmp) == -1)
    return (-1);
  return (0);
}

int
kill_complete(struct kill *msg)
{
  if (!msg->weapon_set)
    return (-1);
  if (!msg->action_set)
    return (-1);
  return (0);
}

int
evtag_unmarshal_kill(struct evbuffer *evbuf, ev_uint32_t need_tag, struct kill *msg)
{
  ev_uint32_t tag;
  int res = -1;

  struct evbuffer *tmp = evbuffer_new();

  if (evtag_unmarshal(evbuf, &tag, tmp) == -1 || tag != need_tag)
    goto error;

  if (kill_unmarshal(msg, tmp) == -1)
    goto error;

  res = 0;

 error:
  evbuffer_free(tmp);
  return (res);
}

void
evtag_marshal_kill(struct evbuffer *evbuf, ev_uint32_t tag, const struct kill *msg)
{
  struct evbuffer *_buf = evbuffer_new();
  assert(_buf != NULL);
  evbuffer_drain(_buf, -1);
  kill_marshal(_buf, msg);
  evtag_marshal(evbuf, tag, EVBUFFER_DATA(_buf), EVBUFFER_LENGTH(_buf));
  evbuffer_free(_buf);
}


static struct run_access_ __run_base = {
  run_how_assign,
  run_how_get,
  run_some_bytes_assign,
  run_some_bytes_get,
  run_fixed_bytes_assign,
  run_fixed_bytes_get,
};

struct run *
run_new(void)
{
  struct run *tmp;
  if ((tmp = malloc(sizeof(struct run))) == NULL) {
    event_warn("%s: malloc", __func__);
    return (NULL);
  }
  tmp->base = &__run_base;

  tmp->how_data = NULL;
  tmp->how_set = 0;

  tmp->some_bytes_data = NULL;
  tmp->some_bytes_length = 0;
  tmp->some_bytes_set = 0;

  memset(tmp->fixed_bytes_data, 0, sizeof(tmp->fixed_bytes_data));
  tmp->fixed_bytes_set = 0;

  return (tmp);
}




int
run_how_assign(struct run *msg,
    const char * value)
{
  if (msg->how_data != NULL)
    free(msg->how_data);
  if ((msg->how_data = strdup(value)) == NULL)
    return (-1);
  msg->how_set = 1;
  return (0);
}

int
run_some_bytes_assign(struct run *msg, const ev_uint8_t * value, ev_uint32_t len)
{
  if (msg->some_bytes_data != NULL)
    free (msg->some_bytes_data);
  msg->some_bytes_data = malloc(len);
  if (msg->some_bytes_data == NULL)
    return (-1);
  msg->some_bytes_set = 1;
  msg->some_bytes_length = len;
  memcpy(msg->some_bytes_data, value, len);
  return (0);
}

int
run_fixed_bytes_assign(struct run *msg, const ev_uint8_t *value)
{
  msg->fixed_bytes_set = 1;
  memcpy(msg->fixed_bytes_data, value, 24);
  return (0);
}

int
run_how_get(struct run *msg, char * *value)
{
  if (msg->how_set != 1)
    return (-1);
  *value = msg->how_data;
  return (0);
}

int
run_some_bytes_get(struct run *msg, ev_uint8_t * *value, ev_uint32_t *plen)
{
  if (msg->some_bytes_set != 1)
    return (-1);
  *value = msg->some_bytes_data;
  *plen = msg->some_bytes_length;
  return (0);
}

int
run_fixed_bytes_get(struct run *msg, ev_uint8_t **value)
{
  if (msg->fixed_bytes_set != 1)
    return (-1);
  *value = msg->fixed_bytes_data;
  return (0);
}

void
run_clear(struct run *tmp)
{
  if (tmp->how_set == 1) {
    free (tmp->how_data);
    tmp->how_data = NULL;
    tmp->how_set = 0;
  }
  if (tmp->some_bytes_set == 1) {
    free (tmp->some_bytes_data);
    tmp->some_bytes_data = NULL;
    tmp->some_bytes_length = 0;
    tmp->some_bytes_set = 0;
  }
  tmp->fixed_bytes_set = 0;
  memset(tmp->fixed_bytes_data, 0, sizeof(tmp->fixed_bytes_data));
}

void
run_free(struct run *tmp)
{
  if (tmp->how_data != NULL)
      free (tmp->how_data); 
  if (tmp->some_bytes_data != NULL)
      free (tmp->some_bytes_data); 
  free(tmp);
}

void
run_marshal(struct evbuffer *evbuf, const struct run *tmp){
  evtag_marshal_string(evbuf, RUN_HOW, tmp->how_data);
  if (tmp->some_bytes_set) {
    evtag_marshal(evbuf, RUN_SOME_BYTES, tmp->some_bytes_data, tmp->some_bytes_length);
  }
  evtag_marshal(evbuf, RUN_FIXED_BYTES, tmp->fixed_bytes_data, sizeof(tmp->fixed_bytes_data));
}

int
run_unmarshal(struct run *tmp,  struct evbuffer *evbuf)
{
  ev_uint32_t tag;
  while (EVBUFFER_LENGTH(evbuf) > 0) {
    if (evtag_peek(evbuf, &tag) == -1)
      return (-1);
    switch (tag) {

      case RUN_HOW:

        if (tmp->how_set)
          return (-1);
        if (evtag_unmarshal_string(evbuf, RUN_HOW, &tmp->how_data) == -1) {
          event_warnx("%s: failed to unmarshal how", __func__);
          return (-1);
        }
        tmp->how_set = 1;
        break;

      case RUN_SOME_BYTES:

        if (tmp->some_bytes_set)
          return (-1);
        if (evtag_payload_length(evbuf, &tmp->some_bytes_length) == -1)
          return (-1);
        if (tmp->some_bytes_length > EVBUFFER_LENGTH(evbuf))
          return (-1);
        if ((tmp->some_bytes_data = malloc(tmp->some_bytes_length)) == NULL)
          return (-1);
        if (evtag_unmarshal_fixed(evbuf, RUN_SOME_BYTES, tmp->some_bytes_data, tmp->some_bytes_length) == -1) {
          event_warnx("%s: failed to unmarshal some_bytes", __func__);
          return (-1);
        }
        tmp->some_bytes_set = 1;
        break;

      case RUN_FIXED_BYTES:

        if (tmp->fixed_bytes_set)
          return (-1);
        if (evtag_unmarshal_fixed(evbuf, RUN_FIXED_BYTES, tmp->fixed_bytes_data, sizeof(tmp->fixed_bytes_data)) == -1) {
          event_warnx("%s: failed to unmarshal fixed_bytes", __func__);
          return (-1);
        }
        tmp->fixed_bytes_set = 1;
        break;

      default:
        return -1;
    }
  }

  if (run_complete(tmp) == -1)
    return (-1);
  return (0);
}

int
run_complete(struct run *msg)
{
  if (!msg->how_set)
    return (-1);
  if (!msg->fixed_bytes_set)
    return (-1);
  return (0);
}

int
evtag_unmarshal_run(struct evbuffer *evbuf, ev_uint32_t need_tag, struct run *msg)
{
  ev_uint32_t tag;
  int res = -1;

  struct evbuffer *tmp = evbuffer_new();

  if (evtag_unmarshal(evbuf, &tag, tmp) == -1 || tag != need_tag)
    goto error;

  if (run_unmarshal(msg, tmp) == -1)
    goto error;

  res = 0;

 error:
  evbuffer_free(tmp);
  return (res);
}

void
evtag_marshal_run(struct evbuffer *evbuf, ev_uint32_t tag, const struct run *msg)
{
  struct evbuffer *_buf = evbuffer_new();
  assert(_buf != NULL);
  evbuffer_drain(_buf, -1);
  run_marshal(_buf, msg);
  evtag_marshal(evbuf, tag, EVBUFFER_DATA(_buf), EVBUFFER_LENGTH(_buf));
  evbuffer_free(_buf);
}

