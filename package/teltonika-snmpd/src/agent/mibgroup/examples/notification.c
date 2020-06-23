/** @example notification.c
 *  This example shows how to send a notification from inside the
 *  agent.  In this case we do something really boring to decide
 *  whether to send a notification or not: we simply sleep for 30
 *  seconds and send it, then we sleep for 30 more and send it again.
 *  We do this through the snmp_alarm mechanisms (which are safe to
 *  use within the agent.  Don't use the system alarm() call, it won't
 *  work properly).  Normally, you would probably want to do something
 *  to test whether or not to send an alarm, based on the type of mib
 *  module you were creating.
 *
 *  When this module is compiled into the agent (run configure with
 *  --with-mib-modules="examples/notification") then it should send
 *  out traps, which when received by the snmptrapd demon will look
 *  roughly like:
 *
 *  2002-05-08 08:57:05 localhost.localdomain [udp:127.0.0.1:32865]:
 *      sysUpTimeInstance = Timeticks: (3803) 0:00:38.03        snmpTrapOID.0 = OID: netSnmpExampleNotification
 *
 */

/*
 * start be including the appropriate header files
 */
#include <string.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

/*
 * contains prototypes
 */
#include "notification.h"

#define GSM_STRING_LEN	256

char search_result[GSM_STRING_LEN];

int signal_count = 0;
int old_analog_value = 0;

char old_conType[GSM_STRING_LEN] ;
char old_state_in_both[256], old_state_in_act[256], old_state_in_inact[256];
char old_state_oc_both[256], old_state_oc_act[256], old_state_oc_inact[256];
char old_state_out_both[256], old_state_out_act[256], old_state_out_inact[256];
char old_state_rel_both[256], old_state_rel_act[256], old_state_rel_inact[256];
char old_state_4pin_out_both[256], old_state_4pin_out_act[256], old_state_4pin_out_inact[256];
char old_state_4pin_in_both[256], old_state_4pin_in_act[256], old_state_4pin_in_inact[256];

/*
 * our initialization routine
 * (to get called, the function name must match init_FILENAME()
 */
void
init_notification(void)
{
	DEBUGMSGTL(("example_notificaticd rut8on","initializing (setting callback alarm)\n"));
	snmp_alarm_register(5,/* seconds */
						SA_REPEAT,      /* repeat (every 30 seconds). */
						send_example_notification,      /* our callback */
						NULL    /* no callback data needed */
	);
}

int get_parm(char *param, char *buffer, int use)
{
	char cmd[GSM_STRING_LEN];
	FILE* name = NULL;
	int iLength;

	if (use == 0)
		sprintf(cmd, "/usr/sbin/sysget %s 2>/dev/null", param);
	else if (use == 1)
		sprintf(cmd, "/usr/bin/mdcget %s 2>/dev/null", param);
	else if (use == 2)
		sprintf(cmd, "/bin/cat %s 2>/dev/null", param);
	else if (use == 3)
		sprintf(cmd, "/sbin/gpio.sh get  %s 2>/dev/null", param);


	if ((name = popen(cmd, "r")) == NULL)
		return -1;

	if (fgets(buffer, GSM_STRING_LEN, name) == NULL) {
		pclose(name);
		return -1;
	} else {
		iLength = strlen(buffer);
		// trim 'new line' symbol at the end
		if (iLength && buffer[iLength-1] == '\n')
			buffer[iLength-1] = '\0';
	}
	pclose(name);
	return 0;
}

void send_notif(netsnmp_variable_list *notification_vars){
	/*
	* send the trap out.  This will send it to all registered
	* receivers (see the "SETTING UP TRAP AND/OR INFORM DESTINATIONS"
	* section of the snmpd.conf manual page.
	*/
	if (notification_vars != NULL) {
		send_v2trap(notification_vars);
	}

	/*
	* free the created notification variable list
	*/
	DEBUGMSGTL(("example_notification", "cleaning up\n"));
	snmp_free_varbind(notification_vars);
	notification_vars = NULL;
}

/** here we send a SNMP v2 trap (which can be sent through snmpv3 and
 *  snmpv1 as well) and send it out.
 *
 *     The various "send_trap()" calls allow you to specify traps in different
 *  formats.  And the various "trapsink" directives allow you to specify
 *  destinations to receive different formats.
 *  But *all* traps are sent to *all* destinations, regardless of how they
 *  were specified.
 *
 *
 *  I.e. it's
 * @verbatim
 *                                           ___  trapsink
 *                                          /
 *      send_easy_trap \___  [  Trap      ] ____  trap2sink
 *                      ___  [ Generator  ]
 *      send_v2trap    /     [            ] ----- informsink
 *                                          \____
 *                                                trapsess
 *
 *  *Not*
 *       send_easy_trap  ------------------->  trapsink
 *       send_v2trap     ------------------->  trap2sink
 *       ????            ------------------->  informsink
 *       ????            ------------------->  trapsess
 * @endverbatim
 */
void
send_example_notification(unsigned int clientreg, void *clientarg)
{
	/*
	* define the OID for the notification we're going to send
	* NET-SNMP-EXAMPLES-MIB::netSnmpExampleHeartbeatNotification
	*/

	oid	signal_str_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 4, 1 };
	size_t	signal_str_oid_len = OID_LENGTH(signal_str_oid);
	static	u_long count = 0;

	oid	connection_type_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 4, 2 };
	size_t	connection_type_oid_len = OID_LENGTH(connection_type_oid);

	oid	digital_input_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 4, 3 };
	size_t	digital_input_oid_len = OID_LENGTH(digital_input_oid);

	oid	digital_input_state_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 4, 4 };
	size_t	digital_input_state_oid_len = OID_LENGTH(digital_input_state_oid);

	oid	digital_oc_input_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 4, 5 };
	size_t	digital_oc_input_oid_len = OID_LENGTH(digital_oc_input_oid);

	oid	digital_oc_input_state_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 4, 6 };
	size_t	digital_oc_input_state_oid_len = OID_LENGTH(digital_oc_input_state_oid);

	oid	analog_input_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 4, 7 };
	size_t	analog_input_oid_len = OID_LENGTH(analog_input_oid);

	oid	analog_input_state_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 4, 8 };
	size_t	analog_input_state_oid_len = OID_LENGTH(analog_input_state_oid);

	oid	digital_output_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 4, 9 };
	size_t	digital_output_oid_len = OID_LENGTH(digital_output_oid);

	oid	digital_output_state_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 4, 10 };
	size_t	digital_output_state_oid_len = OID_LENGTH(digital_output_state_oid);

	oid	digital_relay_output_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 4, 11 };
	size_t	digital_relay_output_oid_len = OID_LENGTH(digital_relay_output_oid);

	oid	digital_relay_output_state_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 4, 12 };
	size_t	digital_relay_output_state_oid_len = OID_LENGTH(digital_relay_output_state_oid);

	//4PIN I/O
	oid	digital_4pin_input_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 4, 13 };
	size_t	digital_4pin_input_oid_len = OID_LENGTH(digital_4pin_input_oid);

	oid	digital_4pin_input_state_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 4, 14 };
	size_t	digital_4pin_input_state_oid_len = OID_LENGTH(digital_4pin_input_state_oid);
	
	oid	digital_4pin_output_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 4, 15 };
	size_t	digital_4pin_output_oid_len = OID_LENGTH(digital_4pin_output_oid);

	oid	digital_4pin_output_state_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 4, 16 };
	size_t	digital_4pin_output_state_oid_len = OID_LENGTH(digital_4pin_output_state_oid);

	/*
	* In the notification, we have to assign our notification OID to
	* the snmpTrapOID.0 object. Here is it's definition.
	*/
	oid		objid_snmptrap[] = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };
	size_t	objid_snmptrap_len = OID_LENGTH(objid_snmptrap);

	/*
	* define the OIDs for the varbinds we're going to include
	*  with the notification -
	* NET-SNMP-EXAMPLES-MIB::netSnmpExampleHeartbeatRate  and
	* NET-SNMP-EXAMPLES-MIB::netSnmpExampleHeartbeatName
	*/
//		oid      hbeat_rate_oid[]   = { 1, 3, 6, 1, 4, 1, 8072, 2, 3, 2, 1, 0 };
//		size_t   hbeat_rate_oid_len = OID_LENGTH(hbeat_rate_oid);
//		oid      hbeat_name_oid[]   = { 1, 3, 6, 1, 4, 1, 48690, 1, 1, 10, 0  };
//		size_t   hbeat_name_oid_len = OID_LENGTH(hbeat_name_oid);

	/*
	* here is where we store the variables to be sent in the trap
	*/
	netsnmp_variable_list *notification_vars[20] = {NULL};
	enum Types {
		signal_str=0,
		conn_type,
		dig_input,
		dig_oc_input,
		an_input,
		dig_output,
		dig_rel_output,
		dig_4pin_input,
		dig_4pin_output,
	};

	DEBUGMSGTL(("example_notification", "defining the trap\n"));

	/*
	* add in the trap definition object
	*/

	FILE *pCMD;
	char allrules[1000], temp[512];
	char sigStr[10];
	char sigEnb[2];
	char conEnb[2];

	if ((pCMD = popen("/sbin/get_rules.sh", "r"))) {
		allrules[0] = '\0';
		while (fgets(temp, sizeof(temp), pCMD)){
			strncat(allrules, temp, sizeof(allrules));
		}
		//allrules[strlen(allrules)-1] = 0; //remove new line
		pclose(pCMD);
	}

	//=========================parsinimas config rules

	char *token, *token_next, enable[50], en_value[50], action[50], ac_value[50], volts[50], vol_value[50], state[50], st_value[50], analog[50], analog_value[50], signal[50], sig_value[50];
	char *search = "\n";

	/* get the first token */
	token = strtok(allrules, search);

	/* walk through other tokens */
	while( token != NULL )
	{
		/*parsinamas antras lygis*/

		sscanf( token, "%s %s %s %s %s %s %s %s %s %s %s %s", enable, en_value, action, ac_value, volts, vol_value, state, st_value, analog, analog_value, signal, sig_value );

		/*parsinas iki | toliau parsina su sscanf*/
// 			snprintf(naujas, sizeof(naujas), "%s", token);
// 			naujas[sizeof(naujas) - 1] = '\0';

		if ( strncmp( enable, "enabled", sizeof("enable") ) != 0 || strncmp( en_value, "1", sizeof("1") ) !=0 ){
			token = strtok(NULL, search);
			continue;
		}
			//signalo stiprumo trepas
		else if ( strncmp( action, "action", sizeof("action")) == 0 && strncmp( ac_value, "sigEnb", sizeof("sigEnb") ) == 0){
			if (get_parm("signal", search_result, 0) == 0) {
				if (atoi(search_result) < atoi(sig_value)){
					//printf("Signal string = %s and int %d \n", search_result, atoi(search_result));
					if (signal_count < 1) {
						++signal_count;
						snmp_varlist_add_variable(&notification_vars[signal_str],
								/*
								* the snmpTrapOID.0 variable
								*/
												  objid_snmptrap, objid_snmptrap_len,
								/*
								* value type is an OID
								*/
												  ASN_OBJECT_ID,
								/*
								* value contents is our notification OID
								*/
												  (u_char *) signal_str_oid,
								/*
								* size in bytes = oid length * sizeof(oid)
								*/
												  signal_str_oid_len * sizeof(oid));


					}
					send_notif(notification_vars[signal_str]);
				} else {
					signal_count = 0;
				}
			}
		}
			//trepas kai keiciasi konekcijos tipas
		else if (strncmp( action, "action", sizeof("action")) == 0 && strncmp( ac_value, "conEnb", sizeof("conEnb") ) == 0){
			if (get_parm("conntype", search_result, 0) == 0) {
				if (old_conType[0] != '\0' && strncmp(search_result, old_conType, sizeof(search_result)) != 0) {
					// printf("Trap\n");
					snmp_varlist_add_variable(&notification_vars[conn_type],
							/*
							* the snmpTrapOID.0 variable
							*/
											  objid_snmptrap, objid_snmptrap_len,
							/*
							* value type is an OID
							*/
											  ASN_OBJECT_ID,
							/*
							* value contents is our notification OID
							*/
											  (u_char *) connection_type_oid,
							/*
							* size in bytes = oid length * sizeof(oid)
							*/
											  connection_type_oid_len * sizeof(oid));
					send_notif(notification_vars[conn_type]);
				}
			}

			strncpy(old_conType, search_result, sizeof(search_result));
		}
			//trapas del digital input keitimosi
		else if (strncmp( action, "action", sizeof("action")) == 0 && strncmp( ac_value, "digIn", sizeof("digIn") ) == 0){
			/*kai pasirinkta both*/
			if ( strncmp( st_value, "both", sizeof("both")) == 0 ){ //jei trigeris kai pasikeicia busena
				if (get_parm("DIN1", search_result, 3) == 0) {
					if (old_state_in_both[0] != '\0' && strncmp(search_result, old_state_in_both, sizeof(search_result)) != 0 ) {
						//printf("digital input\n");
						snmp_varlist_add_variable(&notification_vars[dig_input],
								/*
								* the snmpTrapOID.0 variable
								*/
												  objid_snmptrap, objid_snmptrap_len,
								/*
								* value type is an OID
								*/
												  ASN_OBJECT_ID,
								/*
								* value contents is our notification OID
								*/
												  (u_char *) digital_input_oid,
								/*
								* size in bytes = oid length * sizeof(oid)
								*/
												  digital_input_oid_len * sizeof(oid));

						snmp_varlist_add_variable(&notification_vars[dig_input], digital_input_state_oid, digital_input_state_oid_len,
												  ASN_OCTET_STR, search_result, 1);
						send_notif(notification_vars[dig_input]);
					}
				}

				strncpy(old_state_in_both, search_result, sizeof(search_result));
			}			/*kai pasirinkta active*/
			else if ( strncmp( st_value, "active", sizeof("active")) == 0 ){ //jei trigeris kai pasikeicia busena i active
				if (get_parm("DIN1", search_result, 3) == 0) {
					if (old_state_in_act[0] != '\0' && strncmp(search_result, old_state_in_act, sizeof(search_result)) != 0 && strncmp(old_state_in_act, "0", sizeof("0")) == 0 ){
						// printf("Trap\n");
						snmp_varlist_add_variable(&notification_vars[dig_input],
								/*
								* the snmpTrapOID.0 variable
								*/
												  objid_snmptrap, objid_snmptrap_len,
								/*
								* value type is an OID
								*/
												  ASN_OBJECT_ID,
								/*
								* value contents is our notification OID
								*/
												  (u_char *) digital_input_oid,
								/*
								* size in bytes = oid length * sizeof(oid)
								*/
												  digital_input_oid_len * sizeof(oid));
						snmp_varlist_add_variable(&notification_vars[dig_input], digital_input_state_oid, digital_input_state_oid_len,
												  ASN_OCTET_STR, search_result, 1);
						send_notif(notification_vars[dig_input]);
					}
				}

				strncpy(old_state_in_act, search_result, sizeof(search_result));
			}
				/*kai pasirinkta inactive*/
			else if ( strncmp( st_value, "inactive", sizeof("inactive")) == 0 ){ //jei trigeris kai pasikeicia busena i inactive
				if (get_parm("DIN1", search_result, 3) == 0) {
					if (old_state_in_inact[0] != '\0' && strncmp(search_result, old_state_in_inact, sizeof(search_result)) != 0 && strncmp(old_state_in_inact, "1", sizeof("1")) == 0 ){
						// printf("Trap\n");
						snmp_varlist_add_variable(&notification_vars[dig_input],
								/*
								* the snmpTrapOID.0 variable
								*/
												  objid_snmptrap, objid_snmptrap_len,
								/*
								* value type is an OID
								*/
												  ASN_OBJECT_ID,
								/*
								* value contents is our notification OID
								*/
												  (u_char *) digital_input_oid,
								/*
								* size in bytes = oid length * sizeof(oid)
								*/
												  digital_input_oid_len * sizeof(oid));
						snmp_varlist_add_variable(&notification_vars[dig_input], digital_input_state_oid, digital_input_state_oid_len,
												  ASN_OCTET_STR, search_result, 1);
						send_notif(notification_vars[dig_input]);
					}

				}
				strncpy(old_state_in_inact, search_result, sizeof(search_result));
			}
		}
			//trapas del digital oc input keitimosi
		else if (strncmp( action, "action", sizeof("action")) == 0 && strncmp( ac_value, "digOCIn", sizeof("digOCIn") ) == 0){
			/*kai pasirinkta both*/
			if ( strncmp( st_value, "both", sizeof("both")) == 0 ){ //jei trigeris kai pasikeicia busena
				if (get_parm("DIN2", search_result, 3) == 0) {
					if (old_state_oc_both[0] != '\0' && strncmp(search_result, old_state_oc_both, sizeof(search_result)) != 0) {
						//printf("digital oc\n");
						snmp_varlist_add_variable(&notification_vars[dig_oc_input],
								/*
								* the snmpTrapOID.0 variable
								*/
												  objid_snmptrap, objid_snmptrap_len,
								/*
								* value type is an OID
								*/
												  ASN_OBJECT_ID,
								/*
								* value contents is our notification OID
								*/
												  (u_char *) digital_oc_input_oid,
								/*
								* size in bytes = oid length * sizeof(oid)
								*/
												  digital_oc_input_oid_len * sizeof(oid));
						snmp_varlist_add_variable(&notification_vars[dig_oc_input], digital_oc_input_state_oid, digital_oc_input_state_oid_len,
												  ASN_OCTET_STR, search_result, 1);
						send_notif(notification_vars[dig_oc_input]);

					}
				}
				strncpy(old_state_oc_both, search_result, sizeof(search_result));
			}
				/*kai pasirinkta active*/
			else if ( strncmp( st_value, "active", sizeof("active")) == 0 ){ //jei trigeris kai pasikeicia busena i active
				if (get_parm("DIN2", search_result, 3) == 0) {
					if (old_state_oc_act[0] != '\0' && strncmp(search_result, old_state_oc_act, sizeof(search_result)) != 0 && strncmp(old_state_oc_act, "0", sizeof("0")) == 0 ){
						// printf("Trap\n");
						snmp_varlist_add_variable(&notification_vars[dig_oc_input],
								/*
								* the snmpTrapOID.0 variable
								*/
												  objid_snmptrap, objid_snmptrap_len,
								/*
								* value type is an OID
								*/
												  ASN_OBJECT_ID,
								/*
								* value contents is our notification OID
								*/
												  (u_char *) digital_oc_input_oid,
								/*
								* size in bytes = oid length * sizeof(oid)
								*/
												  digital_oc_input_oid_len * sizeof(oid));
						snmp_varlist_add_variable(&notification_vars[dig_oc_input], digital_oc_input_state_oid, digital_oc_input_state_oid_len,
												  ASN_OCTET_STR, search_result, 1);
						send_notif(notification_vars[dig_oc_input]);
					}
				}
				strncpy(old_state_oc_act, search_result, sizeof(search_result));
			}
				/*kai pasirinkta inactive*/
			else if ( strncmp( st_value, "inactive", sizeof("inactive")) == 0 ){ //jei trigeris kai pasikeicia busena i inactive
				if (get_parm("DIN2", search_result, 3) == 0) {
					if (old_state_oc_inact[0] != '\0' && strncmp(search_result, old_state_oc_inact, sizeof(search_result)) != 0 && strncmp(old_state_oc_inact, "1", sizeof("1")) == 0 ){
						snmp_varlist_add_variable(&notification_vars[dig_oc_input], objid_snmptrap, objid_snmptrap_len,
												  ASN_OBJECT_ID, (u_char *) digital_oc_input_oid, digital_oc_input_oid_len * sizeof(oid));
						snmp_varlist_add_variable(&notification_vars[dig_oc_input], digital_oc_input_state_oid, digital_oc_input_state_oid_len,
												  ASN_OCTET_STR, search_result, 1);
						send_notif(notification_vars[dig_oc_input]);
					}
				}
				strncpy(old_state_oc_inact, search_result, sizeof(search_result));
			}
		}
// 		//trapas del analog input keitimosi
		else if (strncmp( action, "action", sizeof("action")) == 0 && strncmp( ac_value, "analog", sizeof("analog") ) == 0){
			if ( analog && strncmp( analog_value, "both", sizeof("both")) == 0 ){
				if (get_parm("/sys/class/hwmon/hwmon0/device/in0_input", search_result, 2) == 0) {
					double result;
					result = atof(vol_value)*1000;

					// analog value higher then config
					if (atoi(search_result) > result && old_analog_value < result || atoi(search_result) > result && signal_count == 0){
						// 						printf("Signal string = %s and int %d \n", search_result, atoi(search_result));
						snmp_varlist_add_variable(&notification_vars[an_input], objid_snmptrap, objid_snmptrap_len,
												  ASN_OBJECT_ID, (u_char *) analog_input_oid, analog_input_oid_len * sizeof(oid));
						snmp_varlist_add_variable(&notification_vars[an_input], analog_input_state_oid, analog_input_state_oid_len,
												  ASN_OCTET_STR, "higher", sizeof("higher"));
						send_notif(notification_vars[an_input]);
						signal_count = 1;

					} else if (atoi(search_result) < result && old_analog_value > result || atoi(search_result) < result && signal_count == 0){
						// 					printf("Signal string = %s and int %d \n", search_result, atoi(search_result));
						snmp_varlist_add_variable(&notification_vars[an_input], objid_snmptrap, objid_snmptrap_len,
												  ASN_OBJECT_ID, (u_char *) analog_input_oid,	analog_input_oid_len * sizeof(oid));
						snmp_varlist_add_variable(&notification_vars[an_input], analog_input_state_oid, analog_input_state_oid_len,
												  ASN_OCTET_STR, "lower", sizeof("lower"));
						send_notif(notification_vars[an_input]);
						signal_count = 1;
					}
					old_analog_value = atoi(search_result);
				}
			}
			else if ( analog && strncmp( analog_value, "higher", sizeof("higher")) == 0 ){
				if (get_parm("/sys/class/hwmon/hwmon0/device/in0_input", search_result, 2) == 0) {
					double result;
					result = atof(vol_value)*1000;

					// analog value higher then config
					if (atoi(search_result) > result && old_analog_value < result || atoi(search_result) > result && signal_count == 0){
						// 						printf("Signal string = %s and int %d \n", search_result, atoi(search_result));
						snmp_varlist_add_variable(&notification_vars[an_input],	objid_snmptrap, objid_snmptrap_len,	ASN_OBJECT_ID,
												  (u_char *) analog_input_oid, analog_input_oid_len * sizeof(oid));
						snmp_varlist_add_variable(&notification_vars[an_input], analog_input_state_oid, analog_input_state_oid_len,
												  ASN_OCTET_STR, "higher", sizeof("higher"));
						send_notif(notification_vars[an_input]);
						signal_count = 1;
					}
					old_analog_value = atoi(search_result);
				}
			}
			else if ( analog && strncmp( analog_value, "lower", sizeof("lower")) == 0 ){
				if (get_parm("/sys/class/hwmon/hwmon0/device/in0_input", search_result, 2) == 0) {
					double result;
					result = atof(vol_value)*1000;

					// analog value higher then config
					if (atoi(search_result) < result && old_analog_value > result || atoi(search_result) < result && signal_count == 0){
						// 						printf("Signal string = %s and int %d \n", search_result, atoi(search_result));
						snmp_varlist_add_variable(&notification_vars[an_input],	objid_snmptrap, objid_snmptrap_len,
												  ASN_OBJECT_ID, (u_char *) analog_input_oid,	analog_input_oid_len * sizeof(oid));
						snmp_varlist_add_variable(&notification_vars[an_input], analog_input_state_oid, analog_input_state_oid_len,
												  ASN_OCTET_STR, "lower", sizeof("lower"));
						send_notif(notification_vars[an_input]);
						signal_count = 1;
					}
					old_analog_value = atoi(search_result);
				}
			}
		}
			//trapas del digital output keitimosi
		else if (strncmp( action, "action", sizeof("action")) == 0 && strncmp( ac_value, "digOut", sizeof("digOut") ) == 0){
			/*kai pasirinkta both*/
			if ( strncmp( st_value, "both", sizeof("both")) == 0 ){ //jei trigeris kai pasikeicia busena
				if (get_parm("DOUT1", search_result, 3) == 0) {
					if (old_state_out_both[0] != '\0' && strncmp(search_result, old_state_out_both, sizeof(search_result)) != 0 ) {
						//printf("digital output\n");
						snmp_varlist_add_variable(&notification_vars[dig_output],	objid_snmptrap, objid_snmptrap_len,
												  ASN_OBJECT_ID, (u_char *) digital_output_oid,	digital_output_oid_len * sizeof(oid));
						snmp_varlist_add_variable(&notification_vars[dig_output], digital_output_state_oid, digital_output_state_oid_len,
												  ASN_OCTET_STR, search_result, 1);
						send_notif(notification_vars[dig_output]);

					}
				}
				strncpy(old_state_out_both, search_result, sizeof(search_result));
			}
				/*kai pasirinkta active*/
			else if ( strncmp( st_value, "active", sizeof("active")) == 0 ){ //jei trigeris kai pasikeicia busena i active
				if (get_parm("DOUT1", search_result, 3) == 0) {
					if (old_state_out_act[0] != '\0' && strncmp(search_result, old_state_out_act, sizeof(search_result)) != 0 && strncmp(old_state_out_act, "0", sizeof("0")) == 0 ){
						snmp_varlist_add_variable(&notification_vars[dig_output],	objid_snmptrap, objid_snmptrap_len,
												  ASN_OBJECT_ID,	(u_char *) digital_output_oid,	digital_output_oid_len * sizeof(oid));
						snmp_varlist_add_variable(&notification_vars[dig_output], digital_output_state_oid, digital_output_state_oid_len,
												  ASN_OCTET_STR, search_result, 1);
						send_notif(notification_vars[dig_output]);
					}
				}
				strncpy(old_state_out_act, search_result, sizeof(search_result));
			}
				/*kai pasirinkta inactive*/
			else if ( strncmp( st_value, "inactive", sizeof("inactive")) == 0 ){ //jei trigeris kai pasikeicia busena i inactive
				if (get_parm("DOUT1", search_result, 3) == 0) {
					if (old_state_out_inact[0] != '\0' && strncmp(search_result, old_state_out_inact, sizeof(search_result)) != 0 && strncmp(old_state_out_inact, "1", sizeof("1")) == 0 ){
						snmp_varlist_add_variable(&notification_vars[dig_output],	objid_snmptrap, objid_snmptrap_len,
												  ASN_OBJECT_ID, (u_char *) digital_output_oid,	digital_output_oid_len * sizeof(oid));
						snmp_varlist_add_variable(&notification_vars[dig_output], digital_output_state_oid, digital_output_state_oid_len,
												  ASN_OCTET_STR, search_result, 1);
						send_notif(notification_vars[dig_output]);
					}
				}
				strncpy(old_state_out_inact, search_result, sizeof(search_result));
			}
		}
		
			//trapas del digital relay output keitimosi
		else if (strncmp( action, "action", sizeof("action")) == 0 && strncmp( ac_value, "digRelayOut", sizeof("digRelayOut") ) == 0){
			//printf("Digital Relay Output\n");
			/*kai pasirinkta both*/
			if ( strncmp( st_value, "both", sizeof("both")) == 0 ){ //jei trigeris kai pasikeicia busena
				if (get_parm("DOUT2", search_result, 3) == 0) {
					if (old_state_rel_both[0] != '\0' && strncmp(search_result, old_state_rel_both, sizeof(search_result)) != 0 ) {
						//printf("digital relay output\n");
						snmp_varlist_add_variable(&notification_vars[dig_rel_output],	objid_snmptrap, objid_snmptrap_len,
												  ASN_OBJECT_ID, (u_char *) digital_relay_output_oid,	digital_relay_output_oid_len * sizeof(oid));
						snmp_varlist_add_variable(&notification_vars[dig_rel_output], digital_relay_output_state_oid, digital_relay_output_state_oid_len,
												  ASN_OCTET_STR, search_result, 1);
						send_notif(notification_vars[dig_rel_output]);
					}
				}
				strncpy(old_state_rel_both, search_result, sizeof(search_result));
			}
				/*kai pasirinkta active*/
			else	if ( strncmp( st_value, "active", sizeof("active")) == 0 ){ //jei trigeris kai pasikeicia busena i active
				if (get_parm("DOUT2", search_result, 3) == 0) {
					if (old_state_rel_act[0] != '\0' && strncmp(search_result, old_state_rel_act, sizeof(search_result)) != 0 && strncmp(old_state_rel_act, "0", sizeof("0")) == 0 ){
						snmp_varlist_add_variable(&notification_vars[dig_rel_output],	objid_snmptrap, objid_snmptrap_len,	ASN_OBJECT_ID,
												  (u_char *) digital_relay_output_oid, digital_relay_output_oid_len * sizeof(oid));
						snmp_varlist_add_variable(&notification_vars[dig_rel_output], digital_relay_output_state_oid, digital_relay_output_state_oid_len,
												  ASN_OCTET_STR, search_result, 1);
						send_notif(notification_vars[dig_rel_output]);
					}
				}
				strncpy(old_state_rel_act, search_result, sizeof(search_result));
			}
				/*kai pasirinkta inactive*/
			else if ( strncmp( st_value, "inactive", sizeof("inactive")) == 0 ){ //jei trigeris kai pasikeicia busena i inactive
				if (get_parm("DOUT2", search_result, 3) == 0) {
					if (old_state_rel_inact[0] != '\0' && strncmp(search_result, old_state_rel_inact, sizeof(search_result)) != 0 && strncmp(old_state_rel_inact, "1", sizeof("1")) == 0 ){
						snmp_varlist_add_variable(&notification_vars[dig_rel_output],	objid_snmptrap, objid_snmptrap_len,
												  ASN_OBJECT_ID, (u_char *) digital_relay_output_oid,	digital_relay_output_oid_len * sizeof(oid));
						snmp_varlist_add_variable(&notification_vars[dig_rel_output], digital_relay_output_state_oid, digital_relay_output_state_oid_len,
												  ASN_OCTET_STR, search_result, 1);
						send_notif(notification_vars[dig_rel_output]);
					}
				}
				strncpy(old_state_rel_inact, search_result, sizeof(search_result));
			}
		}
		
		//--------DIGITAL 4PIN OUTPUT TRAP--------//
		else if (strncmp( action, "action", sizeof("action")) == 0 && strncmp( ac_value, "dig4PinOut", sizeof("dig4PinOut") ) == 0){
			
			//kai pasirinkta BOTH
			if ( strncmp( st_value, "both", sizeof("both")) == 0 ){ //jei trigeris kai pasikeicia busena
				if (get_parm("DOUT3", search_result, 3) == 0) {
					if (old_state_4pin_out_both[0] != '\0' && strncmp(search_result, old_state_4pin_out_both, sizeof(search_result)) != 0 ) {
						snmp_varlist_add_variable(&notification_vars[dig_4pin_output],	objid_snmptrap, objid_snmptrap_len,
												  ASN_OBJECT_ID, (u_char *) digital_4pin_output_oid,	digital_4pin_output_oid_len * sizeof(oid));
						snmp_varlist_add_variable(&notification_vars[dig_4pin_output], digital_4pin_output_state_oid, digital_4pin_output_state_oid_len,
												  ASN_OCTET_STR, search_result, 1);
						send_notif(notification_vars[dig_4pin_output]);
					}
				}
				strncpy(old_state_4pin_out_both, search_result, sizeof(search_result));
			}
			
			//pasirinkta ACTIVE
			else if(strncmp( st_value, "active", sizeof("active")) == 0 ){
				if (get_parm("DOUT3", search_result, 3) == 0) {
					if (old_state_4pin_out_act[0] != '\0' && strncmp(search_result, old_state_4pin_out_act, sizeof(search_result)) != 0 && strncmp(old_state_4pin_out_act, "0", sizeof("0")) == 0 ) {
						snmp_varlist_add_variable(&notification_vars[dig_4pin_output],	objid_snmptrap, objid_snmptrap_len,
												  ASN_OBJECT_ID, (u_char *) digital_4pin_output_oid,	digital_4pin_output_oid_len * sizeof(oid));
						snmp_varlist_add_variable(&notification_vars[dig_4pin_output], digital_4pin_output_state_oid, digital_4pin_output_state_oid_len,
												  ASN_OCTET_STR, search_result, 1);
						send_notif(notification_vars[dig_4pin_output]);
					}
				}
				strncpy(old_state_4pin_out_act, search_result, sizeof(search_result));
			}

			//pasirinkta INACTIVE
			else if(strncmp( st_value, "inactive", sizeof("inactive")) == 0 ){
				if (get_parm("DOUT3", search_result, 3) == 0) {
					if (old_state_4pin_out_inact[0] != '\0' && strncmp(search_result, old_state_4pin_out_inact, sizeof(search_result)) != 0 && strncmp(old_state_4pin_out_inact, "1", sizeof("1")) == 0 ) {
						snmp_varlist_add_variable(&notification_vars[dig_4pin_output],	objid_snmptrap, objid_snmptrap_len,
												  ASN_OBJECT_ID, (u_char *) digital_4pin_output_oid,	digital_4pin_output_oid_len * sizeof(oid));
						snmp_varlist_add_variable(&notification_vars[dig_4pin_output], digital_4pin_output_state_oid, digital_4pin_output_state_oid_len,
												  ASN_OCTET_STR, search_result, 1);
						send_notif(notification_vars[dig_4pin_output]);
					}
				}
				strncpy(old_state_4pin_out_inact, search_result, sizeof(search_result));
			}
		}

		//--------DIGITAL 4PIN INPUT TRAP--------//
		else if (strncmp( action, "action", sizeof("action")) == 0 && strncmp( ac_value, "dig4PinIn", sizeof("dig4PinIn") ) == 0){
			
			//kai pasirinkta BOTH
			if ( strncmp( st_value, "both", sizeof("both")) == 0 ){ 
				if (get_parm("DIN3", search_result, 3) == 0) {
					if (old_state_4pin_in_both[0] != '\0' && strncmp(search_result, old_state_4pin_in_both, sizeof(search_result)) != 0 ) {
						snmp_varlist_add_variable(&notification_vars[dig_4pin_input],	objid_snmptrap, objid_snmptrap_len,
												  ASN_OBJECT_ID, (u_char *) digital_4pin_input_oid,	digital_4pin_input_oid_len * sizeof(oid));
						snmp_varlist_add_variable(&notification_vars[dig_4pin_input], digital_4pin_input_state_oid, digital_4pin_input_state_oid_len,
												  ASN_OCTET_STR, search_result, 1);
						send_notif(notification_vars[dig_4pin_input]);
					}
				}
				strncpy(old_state_4pin_in_both, search_result, sizeof(search_result));
			}
			
			//pasirinkta ACTIVE
			else if(strncmp( st_value, "active", sizeof("active")) == 0 ){
				if (get_parm("DIN3", search_result, 3) == 0) {
					if (old_state_4pin_in_act[0] != '\0' && strncmp(search_result, old_state_4pin_in_act, sizeof(search_result)) != 0 && strncmp(old_state_4pin_in_act, "0", sizeof("0")) == 0 ) {						
						snmp_varlist_add_variable(&notification_vars[dig_4pin_input],	objid_snmptrap, objid_snmptrap_len,
												  ASN_OBJECT_ID, (u_char *) digital_4pin_input_oid,	digital_4pin_input_oid_len * sizeof(oid));
						snmp_varlist_add_variable(&notification_vars[dig_4pin_input], digital_4pin_input_state_oid, digital_4pin_input_state_oid_len,
												  ASN_OCTET_STR, search_result, 1);
						send_notif(notification_vars[dig_4pin_input]);
					}
				}
				strncpy(old_state_4pin_in_act, search_result, sizeof(search_result));
			}

			//pasirinkta INACTIVE
			else if(strncmp( st_value, "inactive", sizeof("inactive")) == 0 ){
				if (get_parm("DIN3", search_result, 3) == 0) {
					if (old_state_4pin_in_inact[0] != '\0' && strncmp(search_result, old_state_4pin_in_inact, sizeof(search_result)) != 0 && strncmp(old_state_4pin_in_inact, "1", sizeof("1")) == 0 ) {
						snmp_varlist_add_variable(&notification_vars[dig_4pin_input],	objid_snmptrap, objid_snmptrap_len,
												  ASN_OBJECT_ID, (u_char *) digital_4pin_input_oid,	digital_4pin_input_oid_len * sizeof(oid));
						snmp_varlist_add_variable(&notification_vars[dig_4pin_input], digital_4pin_input_state_oid, digital_4pin_input_state_oid_len,
												  ASN_OCTET_STR, search_result, 1);
						send_notif(notification_vars[dig_4pin_input]);
					}
				}
				strncpy(old_state_4pin_in_inact, search_result, sizeof(search_result));
			}
		}
		token = strtok(NULL, search);
	}

}