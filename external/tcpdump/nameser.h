
/* @(#) $Header: /tcpdump/master/tcpdump/nameser.h,v 1.14.4.2 2006/11/10 03:15:35 guy Exp $ (LBL) */

#ifndef _NAMESER_H_
#define	_NAMESER_H_

#include <sys/types.h>

#define PACKETSZ	512		/* maximum packet size */
#define MAXDNAME	256		/* maximum domain name */
#define MAXCDNAME	255		/* maximum compressed domain name */
#define MAXLABEL	63		/* maximum length of domain label */
	/* Number of bytes of fixed size data in query structure */
#define QFIXEDSZ	4
	/* number of bytes of fixed size data in resource record */
#define RRFIXEDSZ	10

#define NAMESERVER_PORT	53

#define MULTICASTDNS_PORT	5353

#define QUERY		0x0		/* standard query */
#define IQUERY		0x1		/* inverse query */
#define STATUS		0x2		/* nameserver status query */
#if 0
#define xxx		0x3		/* 0x3 reserved */
#endif
	/* non standard - supports ALLOW_UPDATES stuff from Mike Schwartz */
#define UPDATEA		0x9		/* add resource record */
#define UPDATED		0xa		/* delete a specific resource record */
#define UPDATEDA	0xb		/* delete all named resource record */
#define UPDATEM		0xc		/* modify a specific resource record */
#define UPDATEMA	0xd		/* modify all named resource record */

#define ZONEINIT	0xe		/* initial zone transfer */
#define ZONEREF		0xf		/* incremental zone referesh */

#ifdef T_NULL
#undef T_NULL
#endif
#ifdef T_OPT
#undef T_OPT
#endif
#ifdef T_UNSPEC
#undef T_UNSPEC
#endif
#ifdef NOERROR
#undef NOERROR
#endif

#define NOERROR		0		/* no error */
#define FORMERR		1		/* format error */
#define SERVFAIL	2		/* server failure */
#define NXDOMAIN	3		/* non existent domain */
#define NOTIMP		4		/* not implemented */
#define REFUSED		5		/* query refused */
	/* non standard */
#define NOCHANGE	0xf		/* update failed to change db */

#define T_A		1		/* host address */
#define T_NS		2		/* authoritative server */
#define T_MD		3		/* mail destination */
#define T_MF		4		/* mail forwarder */
#define T_CNAME		5		/* connonical name */
#define T_SOA		6		/* start of authority zone */
#define T_MB		7		/* mailbox domain name */
#define T_MG		8		/* mail group member */
#define T_MR		9		/* mail rename name */
#define T_NULL		10		/* null resource record */
#define T_WKS		11		/* well known service */
#define T_PTR		12		/* domain name pointer */
#define T_HINFO		13		/* host information */
#define T_MINFO		14		/* mailbox information */
#define T_MX		15		/* mail routing information */
#define T_TXT		16		/* text strings */
#define	T_RP		17		/* responsible person */
#define	T_AFSDB		18		/* AFS cell database */
#define T_X25		19		/* X_25 calling address */
#define T_ISDN		20		/* ISDN calling address */
#define T_RT		21		/* router */
#define	T_NSAP		22		/* NSAP address */
#define	T_NSAP_PTR	23		/* reverse lookup for NSAP */
#define T_SIG		24		/* security signature */
#define T_KEY		25		/* security key */
#define T_PX		26		/* X.400 mail mapping */
#define T_GPOS		27		/* geographical position (withdrawn) */
#define T_AAAA		28		/* IP6 Address */
#define T_LOC		29		/* Location Information */
#define T_NXT		30		/* Next Valid Name in Zone */
#define T_EID		31		/* Endpoint identifier */
#define T_NIMLOC	32		/* Nimrod locator */
#define T_SRV		33		/* Server selection */
#define T_ATMA		34		/* ATM Address */
#define T_NAPTR		35		/* Naming Authority PoinTeR */
#define T_KX		36		/* Key Exchanger */
#define T_CERT		37		/* Certificates in the DNS */
#define T_A6		38		/* IP6 address */
#define T_DNAME		39		/* non-terminal redirection */
#define T_SINK		40		/* unknown */
#define T_OPT		41		/* EDNS0 option (meta-RR) */
#define T_APL		42		/* lists of address prefixes */
#define T_DS		43		/* Delegation Signer */
#define T_SSHFP		44		/* SSH Fingerprint */
#define T_IPSECKEY	45		/* IPsec keying material */
#define T_RRSIG		46		/* new security signature */
#define T_NSEC		47		/* provable insecure information */
#define T_DNSKEY	48		/* new security key */
	/* non standard */
#define T_SPF		99		/* sender policy framework */
#define T_UINFO		100		/* user (finger) information */
#define T_UID		101		/* user ID */
#define T_GID		102		/* group ID */
#define T_UNSPEC	103		/* Unspecified format (binary data) */
#define T_UNSPECA	104		/* "unspecified ascii". Ugly MIT hack */
	/* Query type values which do not appear in resource records */
#define T_TKEY		249		/* Transaction Key [RFC2930] */
#define T_TSIG		250		/* Transaction Signature [RFC2845] */
#define T_IXFR		251		/* incremental transfer [RFC1995] */
#define T_AXFR		252		/* transfer zone of authority */
#define T_MAILB		253		/* transfer mailbox records */
#define T_MAILA		254		/* transfer mail agent records */
#define T_ANY		255		/* wildcard match */


#define C_IN		1		/* the arpa internet */
#define C_CHAOS		3		/* for chaos net (MIT) */
#define C_HS		4		/* for Hesiod name server (MIT) (XXX) */
	/* Query class values which do not appear in resource records */
#define C_ANY		255		/* wildcard match */
#define C_QU		0x8000		/* mDNS QU flag in queries */
#define C_CACHE_FLUSH	0x8000		/* mDNS cache flush flag in replies */

#define CONV_SUCCESS 0
#define CONV_OVERFLOW -1
#define CONV_BADFMT -2
#define CONV_BADCKSUM -3
#define CONV_BADBUFLEN -4

typedef struct {
	u_int16_t id;		/* query identification number */
	u_int8_t  flags1;	/* first byte of flags */
	u_int8_t  flags2;	/* second byte of flags */
	u_int16_t qdcount;	/* number of question entries */
	u_int16_t ancount;	/* number of answer entries */
	u_int16_t nscount;	/* number of authority entries */
	u_int16_t arcount;	/* number of resource entries */
} HEADER;

#define DNS_QR(np)	((np)->flags1 & 0x80)		/* response flag */
#define DNS_OPCODE(np)	((((np)->flags1) >> 3) & 0xF)	/* purpose of message */
#define DNS_AA(np)	((np)->flags1 & 0x04)		/* authoritative answer */
#define DNS_TC(np)	((np)->flags1 & 0x02)		/* truncated message */
#define DNS_RD(np)	((np)->flags1 & 0x01)		/* recursion desired */

#define DNS_RA(np)	((np)->flags2 & 0x80)	/* recursion available */
#define DNS_AD(np)	((np)->flags2 & 0x20)	/* authentic data from named */
#define DNS_CD(np)	((np)->flags2 & 0x10)	/* checking disabled by resolver */
#define DNS_RCODE(np)	((np)->flags2 & 0xF)	/* response code */

#define INDIR_MASK	0xc0	/* 11.... */
#define EDNS0_MASK	0x40	/* 01.... */
#  define EDNS0_ELT_BITLABEL 0x01

struct rrec {
	int16_t	r_zone;			/* zone number */
	int16_t	r_class;		/* class number */
	int16_t	r_type;			/* type number */
	u_int32_t	r_ttl;			/* time to live */
	int	r_size;			/* size of data area */
	char	*r_data;		/* pointer to data */
};

#define GETSHORT(s, cp) { \
	register u_char *t_cp = (u_char *)(cp); \
	(s) = ((u_int16_t)t_cp[0] << 8) | (u_int16_t)t_cp[1]; \
	(cp) += 2; \
}

#define GETLONG(l, cp) { \
	register u_char *t_cp = (u_char *)(cp); \
	(l) = (((u_int32_t)t_cp[0]) << 24) \
	    | (((u_int32_t)t_cp[1]) << 16) \
	    | (((u_int32_t)t_cp[2]) << 8) \
	    | (((u_int32_t)t_cp[3])); \
	(cp) += 4; \
}

#define PUTSHORT(s, cp) { \
	register u_int16_t t_s = (u_int16_t)(s); \
	register u_char *t_cp = (u_char *)(cp); \
	*t_cp++ = t_s >> 8; \
	*t_cp   = t_s; \
	(cp) += 2; \
}

#define PUTLONG(l, cp) { \
	register u_int32_t t_l = (u_int32_t)(l); \
	register u_char *t_cp = (u_char *)(cp); \
	*t_cp++ = t_l >> 24; \
	*t_cp++ = t_l >> 16; \
	*t_cp++ = t_l >> 8; \
	*t_cp   = t_l; \
	(cp) += 4; \
}

#endif /* !_NAMESER_H_ */