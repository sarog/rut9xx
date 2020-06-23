#ifndef MODEM_H
#define MODEM_H

// Supported modems.
typedef enum {
	HE910,
	LE910,
	LE910_V2,
	ME909U,
	EM820W,
	MC7354,
	EC20,
	EC25,
	UC20,
	MT421,
	ME909S,
	ME936,
	ME906S,
	MS2131
} modem_dev;

unsigned int get_modem(void);

#endif
