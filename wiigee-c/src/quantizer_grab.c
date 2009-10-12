/*
 * A demo app which does a kludgy job of capturing input, then tying it into
 * the quantizer to let you quantize live wiimote gestures.
 *
 * Only works on cwiid platforms, which is a bit of a shame.
 *
 * Usage: just run the binary, then hold down trigger to capture, release to
 * analyze.  Prints to stdout.
 *
 * I mentioned kludgy, right?  Good.
 *
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include <cwiid.h>

#include <float.h>
#include <math.h>

#include "util.h"
#include "quantizer.h"

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
  //struct cwiid_state state;	/* wiimote state */
  bdaddr_t bdaddr;	/* bluetooth device address */
  //unsigned char mesg = 0;
  //unsigned char led_state = 0;
  unsigned char rpt_mode = 0;
  //unsigned char rumble = 0;

  /* Make stdout unbuffered, which is useful for piping the output of
   * this program into a timestamping utility, such as tai64n(1) */
  setvbuf(stdout, NULL, _IOLBF, 0);

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


struct acc_report {
  int x,y,z;
  time_t t;
  suseconds_t u;
  struct acc_report *next;
};

struct acc_report accs = { 0,0,0, 0,0, NULL };
struct acc_report *acc_cursor = &accs;

struct acc_report * add_acc_report(struct acc_report *cur_in, int x, int y, int z, time_t t, suseconds_t u) {
  struct acc_report *cur = cur_in;
  cur->next = malloc(sizeof(struct acc_report));
  cur = cur->next;

  cur->x = x;
  cur->y = y;
  cur->z = z;

  cur->t = t;
  cur->u = u;

  cur->next = NULL;

  return cur;
}

void dump_acc_stream(struct acc_report *a) {
  printf("dump(%p)\n", a);
  while(a) {
    printf("  x=%d  y=%d  z=%d ; t=%ld.%ld\n", a->x, a->y, a->z, a->t, a->u);
    a = a->next;
  }
}

struct acc_report *reset_acc_stream(struct acc_report *head) {
  struct acc_report *a = head->next;
  printf("reset(%p)\n", a);
  while(a) {
    struct acc_report *next = a->next;
    free(a);
    a = next;
  }
  head->next = NULL;
  return head;
}

/*
 *
 * Downsample the collected sensor input.
 *
 * We'll treat this as a noisy sensor stream, so we want to downsample to the
 * center of buckets.  We'll interpolate with constant values instead of
 * anything clever, and we'll extend datapoints at the end out to the end of
 * the final bucket to "round out" (instead of doing a partial bucket).
 *
 * We'll shoot for a sample rate of 50Hz for now
 *
 *
 */
struct acc_report * downsample_acc_stream(struct acc_report *head) {
  struct acc_report *output_head = malloc(sizeof(struct acc_report));
  memset(output_head, 0, sizeof(struct acc_report));
  struct acc_report *output_cursor = output_head;

  //long sample_period = 20000; // 1e6us/50samples = 2e4 us/sample
  long sample_period = 100000; // 1e6us/10samples = 1e5 us/sample

  long x,y,z,t; // accumulators
  x = y = z = t = 0;

  struct acc_report *a = head->next;
  if (!a) return output_head; // Short circuit if we have no data

  struct acc_report *next = a->next;

  long t_missing;

  while(next) {
    long dt = (next->t - a->t) * 1000000 + (next->u - a->u);

    t_missing = sample_period - t; // time left to accumulate

    while (t_missing < dt) {
      // spans off into next bucket. need to split off a slice, accumulate, emit, and then accumulate the remainder of x,y,z,t

      x += a->x * t_missing;
      y += a->y * t_missing;
      z += a->z * t_missing;
      t += t_missing;
      dt -= t_missing;

      output_cursor = add_acc_report(output_cursor, x/sample_period,y/sample_period,z/sample_period, 0,0);
      // reset state
      x = y = z = t = 0;
      t_missing = sample_period - t;
    }

    // Now left with a dt == 0 or dt < t_missing, so need to accumulate
    x += a->x * dt;
    y += a->y * dt;
    z += a->z * dt;
    t += dt;

    a = next;
    next = a->next;
  }
  // fill out the final bucket with constant interpolation on t_missing and the last x,y,z reading.  (*a points to the last reading)

  t_missing = sample_period - t;
  x += a->x * t_missing;
  y += a->y * t_missing;
  z += a->z * t_missing;
  t += t_missing;

  output_cursor = add_acc_report(output_cursor, x/sample_period,y/sample_period,z/sample_period, 0,0);
  // reset state
      x = y = z = t = 0;
  t_missing = sample_period - t;

  return output_head;
}

void acc_reports_to_gesture(struct acc_report *head, struct gesture *gesture) {
  struct acc_report *a = head->next;
  printf("reset(%p)\n", a);
  while(a) {
    gesture_append(gesture, a->x, a->y, a->z);
    a = a->next;
  }

  double minacc = DBL_MAX;
  double maxacc = DBL_MIN;

  for (int i = 0; i < gesture->data_len; i++) {
      maxacc = MAX(maxacc, fabs(gesture->data[i].x));
      maxacc = MAX(maxacc, fabs(gesture->data[i].y));
      maxacc = MAX(maxacc, fabs(gesture->data[i].z));

      minacc = MIN(minacc, fabs(gesture->data[i].x));
      minacc = MIN(minacc, fabs(gesture->data[i].y));
      minacc = MIN(minacc, fabs(gesture->data[i].z));
  }

  gesture->maxacc = maxacc;
  gesture->minacc = minacc;
}

void run_quantizer(struct acc_report *head) {
    struct quantizer *quantizer = quantizer_new(8);
    struct gesture *gesture = gesture_new();
    struct observation *observation = NULL;

    // Initialize our gesture object
    acc_reports_to_gesture(head, gesture);

    // Train
    quantizer_trainCenteroids(quantizer, gesture);

    // Get out observation object back
    observation = quantizer_getObservationSequence(quantizer, gesture);

    // Dump it
    for (int i = 0; i < observation->sequence_len; i++) {
        printf("%d\n", observation->sequence[i]);
    }

    quantizer_free(quantizer);
    gesture_free(gesture);
    observation_free(observation);
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
  int i;
  //int valid_source;

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

  if (button_state == 0 && accs.next) {
    dump_acc_stream(&accs);

    printf("\n\nDOWNSAMPLED\n");

    struct acc_report *ds = downsample_acc_stream(&accs);
    dump_acc_stream(ds);

    printf("\n\nQUANTIZED\n");

    run_quantizer(ds);

    reset_acc_stream(ds);
    free(ds);

    acc_cursor = reset_acc_stream(&accs);
  }

	break;
      case CWIID_MESG_ACC:

	if (button_state & 0x0004) {
	  struct timeval now;
	  gettimeofday(&now, NULL);
    /*
	  printf("Acc Report: x=%d, y=%d, z=%d   %ld %ld\n",
		 mesg[i].acc_mesg.acc[CWIID_X],
		 mesg[i].acc_mesg.acc[CWIID_Y],
		 mesg[i].acc_mesg.acc[CWIID_Z],
		 now.tv_sec, now.tv_usec);
    */
    acc_cursor = add_acc_report(
     acc_cursor,
		 (signed char)mesg[i].acc_mesg.acc[CWIID_X],
		 (signed char)mesg[i].acc_mesg.acc[CWIID_Y],
		 (signed char)mesg[i].acc_mesg.acc[CWIID_Z],
		 now.tv_sec, now.tv_usec);
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
