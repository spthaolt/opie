/*
 * Startup functions of wellenreiter
 *
 * $Id: daemon.cc,v 1.18 2003-02-07 17:17:35 max Exp $
 */

#include "config.hh"
#include "daemon.hh"

// temporary solution, will be removed soon
#define MAXCHANNEL 13
int card_type;
char sniffer_device[6];
int channel=0;
int timedout=1;

void chanswitch(int blah)
{
  if(channel >= MAXCHANNEL)
  {
  	channel=1;
  }
  else
  {
	channel++;
  }
  card_set_channel(sniffer_device, channel, card_type);
  timedout=0;
  alarm(1);
}

/* Main function of wellenreiterd */
int main(int argc, char **argv)
{
  int sock, maxfd, retval;
  char buffer[WL_SOCKBUF];
  struct pcap_pkthdr header;
  struct sockaddr_in saddr;
  pcap_t *handletopcap;
  const unsigned char *packet;

  fd_set rset;

  fprintf(stderr, "wellenreiterd %s\n\n", VERSION);
  fprintf(stderr, "(c) 2002 by M-M-M\n\n");

  if(argc < 3)
    usage();

  // removed soon, see above
  signal(SIGALRM, chanswitch);
  alarm(1);
  /* Set sniffer device */
  memset(sniffer_device, 0, sizeof(sniffer_device));
  strncpy(sniffer_device, (char *)argv[1], sizeof(sniffer_device) - 1);

  /* Set card type */
  card_type = atoi(argv[2]);
  if(card_type < 1 || card_type > 4)
    usage();

  if(!card_into_monitormode(&handletopcap, sniffer_device, card_type))
  {
    wl_logerr("Cannot initialize the wireless-card, aborting");
    exit(-1);
  }
  wl_loginfo("Set card into monitor mode");

  /////// following line will be moved to lib as soon as possible ////////////
    if((handletopcap = pcap_open_live(sniffer_device, BUFSIZ, 1, 0, NULL)) == NULL)
    {
      wl_logerr("pcap_open_live() failed: %s", strerror(errno));
      exit(-1);
    }

#ifdef HAVE_PCAP_NONBLOCK
    pcap_setnonblock(handletopcap, 1, NULL);
#endif
      
    ////////////////////////////////////////
 
  /* Setup socket for incoming commands */
  if((sock=wl_setupsock(DAEMONADDR, DAEMONPORT, saddr)) < 0)
  {
    wl_logerr("Cannot setup socket");
    exit(-1);
  } 
  wl_loginfo("Set up socket '%d' for GUI communication", sock);

  FD_ZERO(&rset);

  /* Start main loop */
  wl_loginfo("Starting main loop");
  while(1)
  {

    FD_SET(sock, &rset);
    FD_SET(pcap_fileno(handletopcap), &rset);

    // blah
    timedout=1;

    /* socket or pcap handle bigger? Will be cleaned up, have to check pcap */
    maxfd = (sock > pcap_fileno(handletopcap) ? sock : pcap_fileno(handletopcap)) + 1;

    if(select(maxfd, &rset, NULL, NULL, NULL) < 0 && timedout)
    {
      wl_logerr("Error calling select: %s", strerror(errno));
      break;
    }     

    /* Got data on local socket from GUI */
    if(FD_ISSET(sock, &rset))
    {
      /* Receive data from socket */
      if((retval=wl_recv(&sock, saddr, buffer, sizeof(buffer))) < 0 && timedout)
      {
	wl_logerr("Error trying to read: %s", strerror(errno));
	break;
      }
      else
      {
	/* check type of packet and start function according to it */
	switch(retval)
	{
	  case 98:
	   wl_loginfo("Received STARTSNIFF command");
	   break;
	  case 99:
	      wl_loginfo("Received STOPSNIFF command");
	      break;
	  default:
	      wl_logerr("Received unknown command: %d", retval);
	      break;
	}
      }
    } /* FD_ISSET */

    /* Check pcap lib for packets */
    if(FD_ISSET(pcap_fileno(handletopcap), &rset))
    {

      /* Grab one single packet */
      packet = pcap_next(handletopcap, &header);

      /* process the packet */
      process_packets(&header,*&packet, GUIADDR, GUIPORT);
    }

  } /* while(1) */
  
  close(sock);
  exit(0);
}

void usage(void)
{
  fprintf(stderr, "Usage: wellenreiter <device> <cardtype>\n"      \
	  "\t<device>   = Wirelessdevice (e.g. wlan0)\n" \
	  "\t<cardtype> = Cardtype:\tCisco\t= 1\n"       \
	  "\t\t\t\tNG\t= 2\n"                             \
          "\t\t\t\tHOSTAP\t= 3\n"			  \
	  "\t\t\t\tLUCENT\t= 4\n");
  exit(-1);
}
