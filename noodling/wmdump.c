#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <cwiid.h>

cwiid_mesg_callback_t cwiid_callback;

#define toggle_bit(bf,b)	\
	(bf) = ((bf) & b)		\
	       ? ((bf) & ~(b))	\
	       : ((bf) | (b))



void set_led_state(cwiid_wiimote_t *wiimote, unsigned char led_state);
void set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode);
void print_state(struct cwiid_state *state);

cwiid_err_t err;
void err(cwiid_wiimote_t *wiimote, const char *s, va_list ap)
{
	if (wiimote) printf("%d:", cwiid_get_id(wiimote));
	else printf("-1:");
	vprintf(s, ap);
	printf("\n");
}




int main(int argc, char *argv[]) 
{
  cwiid_wiimote_t *wiimote;	/* wiimote handle */
  struct cwiid_state state;	/* wiimote state */
  bdaddr_t bdaddr;	/* bluetooth device address */
  unsigned char mesg = 0;
  unsigned char led_state = 0;
  unsigned char rpt_mode = 0;
  unsigned char rumble = 0;

  cwiid_set_err(err);

  /* Connect to address given on command-line, if present */
  if (argc > 1) {
    str2ba(argv[1], &bdaddr);
  }
  else {
    bdaddr = *BDADDR_ANY;
  }

  printf("Put Wiimote in discoverable mode now (press 1+2)...\n");
  if (!(wiimote = cwiid_open(&bdaddr, 0))) {
    fprintf(stderr, "Unable to connect to wiimote\n");
    exit(1);
  }
  if (cwiid_set_mesg_callback(wiimote, cwiid_callback)) {
    fprintf(stderr, "Unable to set message callback\n");
    exit(1);
  }

  toggle_bit(rpt_mode, CWIID_RPT_ACC);
  toggle_bit(rpt_mode, CWIID_RPT_BTN);
  set_rpt_mode(wiimote, rpt_mode);


  if (cwiid_enable(wiimote, CWIID_FLAG_MESG_IFC)) {
    fprintf(stderr, "Unable to set message flag\n");
    exit(1);
  }


  while(1) {
    sleep(1);
  }
}


void set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode)
{
	if (cwiid_set_rpt_mode(wiimote, rpt_mode)) {
		fprintf(stderr, "Error setting report mode\n");
	}
}


int button_state;

/* Prototype cwiid_callback with cwiid_callback_t, define it with the actual
 * type - this will cause a compile error (rather than some undefined bizarre
 * behavior) if cwiid_callback_t changes */
/* cwiid_mesg_callback_t has undergone a few changes lately, hopefully this
 * will be the last.  Some programs need to know which messages were received
 * simultaneously (e.g. for correlating accelerometer and IR data), and the
 * sequence number mechanism used previously proved cumbersome, so we just
 * pass an array of messages, all of which were received at the same time.
 * The id is to distinguish between multiple wiimotes using the same callback.
 * */
void cwiid_callback(cwiid_wiimote_t *wiimote, int mesg_count,
                    union cwiid_mesg mesg[], struct timespec *timestamp)
{
  int i, j;
  int valid_source;

  for (i=0; i < mesg_count; i++)
    {
      switch (mesg[i].type) {
      case CWIID_MESG_STATUS:
	printf("Status Report: battery=%d extension=",
	       mesg[i].status_mesg.battery);
	switch (mesg[i].status_mesg.ext_type) {
	case CWIID_EXT_NONE:
	  printf("none");
	  break;
	case CWIID_EXT_NUNCHUK:
	  printf("Nunchuk");
	  break;
	case CWIID_EXT_CLASSIC:
	  printf("Classic Controller");
	  break;
	default:
	  printf("Unknown Extension");
	  break;
	}
	printf("\n");
	break;
      case CWIID_MESG_BTN:
	printf("Button Report: %.4X\n", mesg[i].btn_mesg.buttons);
	button_state = mesg[i].btn_mesg.buttons;
	break;
      case CWIID_MESG_ACC:

	if (button_state & 0x0004) {

	  printf("Acc Report: x=%d, y=%d, z=%d\n",
		 mesg[i].acc_mesg.acc[CWIID_X],
		 mesg[i].acc_mesg.acc[CWIID_Y],
		 mesg[i].acc_mesg.acc[CWIID_Z]);
	}
	break;
      case CWIID_MESG_IR:
	printf("IR Report: elided");
	printf("\n");
	break;
      case CWIID_MESG_NUNCHUK:
	printf("Nunchuk Report elided\n");
	break;
      case CWIID_MESG_CLASSIC:
	printf("Classic Report: btns=%.4X l_stick=(%d,%d) r_stick=(%d,%d) "
	       "l=%d r=%d\n", mesg[i].classic_mesg.buttons,
	       mesg[i].classic_mesg.l_stick[CWIID_X],
	       mesg[i].classic_mesg.l_stick[CWIID_Y],
	       mesg[i].classic_mesg.r_stick[CWIID_X],
	       mesg[i].classic_mesg.r_stick[CWIID_Y],
	       mesg[i].classic_mesg.l, mesg[i].classic_mesg.r);
	break;
      case CWIID_MESG_ERROR:
	if (cwiid_close(wiimote)) {
	  fprintf(stderr, "Error on wiimote disconnect\n");
	  exit(-1);
	}
	exit(0);
	break;
      default:
	printf("Unknown Report");
	break;
      }
    }
}
