/* $Id: cardmode.hh,v 1.2 2002-11-28 00:00:52 mjm Exp $ */

#ifndef CARDMODE_HH
#define CARDMODE_HH

#include <string.h>
#include <stdlib.h>
#include <pcap.h>  
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/bpf.h>

#include "../../libwellenreiter/source/log.hh"

/* Defines, used for the card setup */
#define   DEFAULT_PATH       "/proc/driver/aironet/%s/Config"
#define   CARD_TYPE_CISCO	1
#define   CARD_TYPE_NG	        2
#define   CARD_TYPE_HOSTAP	3

/* only for now, until we have the daemon running */
/*the config file should provide these information */
#define   CARD_TYPE              CARD_TYPE_CISCO
#define   SBIN_PATH	         "/sbin/ifconfig %s promisc up"
#define   WLANCTL_PATH           "/sbin/wlanctl-ng"

/* Prototypes */
int card_into_monitormode (pcap_t **, char *, int);
int card_set_promisc_up (const char *);

#endif /* CARDMODE_HH */
