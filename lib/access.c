/* Copyright (C) 1997-2016 Thorsten Kukuk
   Author: Thorsten Kukuk <kukuk@suse.de>

   The YP Server is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

   The YP Server is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public
   License along with the YP Server; see the file COPYING. If
   not, write to the Free Software Foundation, Inc., 51 Franklin Street,
   Suite 500, Boston, MA 02110-1335, USA. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <netdb.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#if USE_SD_NOTIFY
#include <systemd/sd-daemon.h>
#endif

#include "log_msg.h"
#include "ypserv_conf.h"
#include "access.h"
#include "yp_db.h"
#include "yp.h"

static conffile_t *conf = NULL;
const char *confdir = CONFDIR;

void
load_config (void)
{
  conffile_t *tmp;

  if (conf != NULL)
    {
      log_msg ("Reloading %s/ypserv.conf", confdir);
      while (conf)
	{
	  tmp = conf;
	  conf = conf->next;

	  free (tmp->map);
	  free (tmp);
	}
    }

  conf = load_ypserv_conf (confdir);
}

/* Give a string with the DEFINE description back */
static char *
ypproc_name (int proc)
{
  switch (proc)
    {
    case YPPROC_NULL:
      return "ypproc_null";
    case YPPROC_DOMAIN:
      return "ypproc_domain";
    case YPPROC_DOMAIN_NONACK:
      return "ypproc_domain_nonack";
    case YPPROC_MATCH:
      return "ypproc_match";
    case YPPROC_FIRST:
      return "ypproc_first";
    case YPPROC_NEXT:
      return "ypproc_next";
    case YPPROC_XFR:
      return "ypproc_xfr";
    case YPPROC_CLEAR:
      return "ypproc_clear";
    case YPPROC_ALL:
      return "ypproc_all";
    case YPPROC_MASTER:
      return "ypproc_master";
    case YPPROC_ORDER:
      return "ypproc_order";
    case YPPROC_MAPLIST:
      return "ypproc_maplist";
    case YPPROC_NEWXFR:
      return "ypproc_newxfr";
    default:
      log_msg ("Unknown procedure '%i'", proc);
      return "unknown ?";
    }
}

/* The is_valid_domain function checks the domain specified bye the
   caller to make sure it's actually served by this server.

   Return 1 if the name is a valid domain name served by us, else 0. */
int
is_valid_domain (const char *domain)
{
  struct stat sbuf;

  if (domain == NULL || domain[0] == '\0' ||
      strcmp (domain, "binding") == 0 ||
      strcmp (domain, "..") == 0 ||
      strcmp (domain, ".") == 0 ||
      strchr (domain, '/'))
    return 0;

  if (stat (domain, &sbuf) < 0 || !S_ISDIR (sbuf.st_mode))
    return 0;

  return 1;
}

static struct netbuf *
copy_netbuf (struct netbuf *src)
{
  struct netbuf *dst;

  if (src == NULL)
    return NULL;

  dst = calloc (1, sizeof (struct netbuf));
  if (dst == NULL)
    return NULL;

  dst->buf = malloc (src->len);
  if (dst->buf == NULL)
    {
      free (dst);
      return NULL;
    }

  dst->len = src->len;
  dst->maxlen = src->len;
  memcpy (dst->buf, src->buf, src->len);

  return dst;
}

/* return values:
   0: both IP addresses are identical
   1: the IP addresses are not identical */
static int
cmp_netbuf (struct netbuf *nbuf1, struct netbuf *nbuf2)
{
  if (nbuf1 == NULL || nbuf2 == NULL)
    return 1;

  if (nbuf1->len != nbuf2->len)
    return 1;

  if (memcmp (nbuf1->buf, nbuf2->buf, nbuf1->len) == 0)
    return 0;

  return 1;
}

/* By default, we use the securenet list, to check if the client
   is secure.

   return  1, if request comes from an authorized host
   return  0, if securenets does not allow access from this host
   return -1, if request comes from an unauthorized host
   return -2, if the map name is not valid
   return -3, if the domain is not valid
   return -4, if the map does not exist */
int
is_valid (struct svc_req *rqstp, const char *map, const char *domain)
{
  static struct netbuf *oldaddr = NULL;  /* so we dont log multiple times */
  static int oldstatus = -1;
  struct netconfig *nconf;
  struct netbuf *rqhost;
  struct __rpc_sockinfo si;
  int status;

  if (domain && is_valid_domain (domain) == 0)
    return -3;

  if (map && (map[0] == '\0' || strchr (map ,'/')))
    return -2;

  rqhost = svc_getrpccaller (rqstp->rq_xprt);
  nconf = getnetconfigent (rqstp->rq_xprt->xp_netid);

  status = securenet_host (nconf, rqhost);

  if (!__rpc_nconf2sockinfo(nconf, &si))
    return -1;

  if ((map != NULL) && status && si.si_af == AF_INET)
    {
      struct sockaddr_in *sin = rqhost->buf;
      conffile_t *work;

      work = conf;
      while (work)
	{
	  if ((sin->sin_addr.s_addr & work->netmask.s_addr) == work->network.s_addr)
	    if (strcmp (work->domain, domain) == 0 ||
		strcmp (work->domain, "*") == 0)
	      if (strcmp (work->map, map) == 0 || strcmp (work->map, "*") == 0)
		break;
	  work = work->next;
	}

      if (work != NULL)
	switch (work->security)
	  {
	  case SEC_NONE:
	    break;
	  case SEC_DENY:
	    status = -1;
	    break;
	  case SEC_PORT:
	    if (taddr2port (nconf, rqhost) >= IPPORT_RESERVED)
	      status = -1;
	    break;
	  }
      else if (domain != NULL)
	{
	  /* The map is not in the access list, maybe it
	     has a YP_SECURE key ? */
	  DB_FILE dbp = ypdb_open (domain, map);
	  if (dbp != NULL)
	    {
	      datum key;

	      key.dsize = sizeof ("YP_SECURE") - 1;
	      key.dptr = "YP_SECURE";
	      if (ypdb_exists (dbp, key))
		if (taddr2port (nconf, rqhost) >= IPPORT_RESERVED)
		  status = -1;
	      ypdb_close (dbp);
	    }
          else
              status = -4;
	}
    }

  if (debug_flag)
    {
      char host[INET6_ADDRSTRLEN];

      log_msg ("%sconnect from %s port %d to procedure %s (%s,%s;%d)",
	       status ? "" : "refused ",
	       taddr2ipstr (nconf, rqhost, host, sizeof (host)),
	       taddr2port (nconf, rqhost), ypproc_name (rqstp->rq_proc),
	       domain ? domain : "", map ? map : "", status);
    }
  else
    {
      if ((status < 1 && status != -4) &&
	  (cmp_netbuf (oldaddr, rqhost) || (status != oldstatus)))
	{
	  char host[INET6_ADDRSTRLEN];

	  syslog (LOG_WARNING,
		  "refused connect from %s port %d to procedure %s (%s,%s;%d)",
		  taddr2ipstr (nconf, rqhost, host, sizeof (host)),
		  taddr2port (nconf, rqhost), ypproc_name (rqstp->rq_proc),
		  domain ? domain : "", map ? map : "", status);
	}
    }

  /* Create a copy of the netbuf and free the old one */
  if (oldaddr)
    {
      if (oldaddr->buf)
	free (oldaddr->buf);
      free (oldaddr);
    }
  oldaddr = copy_netbuf (rqhost);
  oldstatus = status;

  freenetconfigent (nconf);

  return status;
}

/* Send a messages to systemd daemon, that inicialization of daemon
   is finished and daemon is ready to accept connections.
   It is a nop if we don't use systemd. */
void
announce_ready()
{
#if USE_SD_NOTIFY
  int result;

  result = sd_notifyf(0, "READY=1\n"
                         "STATUS=Processing requests...\n"
                         "MAINPID=%lu", (unsigned long) getpid());

  /* Return code from sd_notifyf can be ignored, as per sd_notifyf(3).
     However, if we use systemd's native unit file, we need to send
     this message to let systemd know that daemon is ready.
     Thus, we want to know that the call had some issues. */
  if (result < 0)
    log_msg ("sd_notifyf failed: %s\n", strerror(-result));
#endif
}
