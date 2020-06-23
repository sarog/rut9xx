#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
//#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include <time.h>

#define MDCOLL_UNIX_SOCK "/tmp/mdcoll.sock"
#define BUFFER_SIZE 256

// sios dienos
int mdcollectd_get_current_day_tx(char **output, int sim);
// konkrecios dienos
int mdcollectd_get_day_tx(int year, int month, int day, char **output, int sim);
// savaites atgal nuo siandie
int mdcollectd_get_week_period_tx(char **output, int sim);
// sios savaites
int mdcollectd_get_current_week_tx(char **output, int sim);
// sio menesio
int mdcollectd_get_current_month_tx(char **output, int sim);
// menesio atgal nuo siandien
int mdcollectd_get_month_period_tx(char **output, int sim);
// konkretaus menesio
int mdcollectd_get_month_tx(int year, int month, char **output, int sim);
// nuo pasirinkto datos iki pasirinktos, jei neurodoma iki kada imama iki siandie
int mdcollectd_get_from_to_tx(int fyear, int fmonth, int fday, int tyear, int tmonth, int tday, char **output, int sim);

int mdcollectd_get_last24_hours_tx(char **output, int sim);
int mdcollectd_get_last24_hours_rx(char **output, int sim);
int mdcollectd_get_current_day_rx(char **output, int sim);
int mdcollectd_get_day_rx(int year, int month, int day, char **output, int sim);
int mdcollectd_get_week_period_rx(char **output, int sim);
int mdcollectd_get_current_week_rx(char **output, int sim);
int mdcollectd_get_current_month_rx(char **output, int sim);
int mdcollectd_get_month_period_rx(char **output, int sim);
int mdcollectd_get_month_rx(int year, int month, char **output, int sim);
int mdcollectd_get_from_to_rx(int fyear, int fmonth, int fday, int tyear, int tmonth, int tday, char **output, int sim);
int mdcollectd_get_from_to_time_rx(char **output, char *conn_period, unsigned int start, unsigned int end, int sim);
int mdcollectd_get_from_to_time_tx(char **output, char *conn_period, unsigned int start, unsigned int end, int sim);

int start_socket();
int mdcollectd_send(char **output, char *command);
time_t get_stime(int year, int month, int day, int hour, int min, int sec);
int get_sim();
void get_date();
void mdcollectd_delete_table_data(char **output, int sim);
void mdcollectd_delete_all_data(char **output, int sim);


typedef struct{
	int year;
	int month;
	int day;
}Date;
