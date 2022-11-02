#!/bin/sh

logger -t "hotspot" "User ${USER_NAME} logged on."

#Script executed after a session is authorized. Executed with the following environment variables:
#
#DEV                            - The TUN/TAP device.
#ADDR                           - IP Address of hotspot service.
#NET                            - Network of hotspot service.
#MASK                           - Network mask of hotspot service.
#NAS_ID                         - NAS Identifier.
#WISPR_LOCATION_ID              - The radiuslocation ID.
#WISPR_LOCATION_NAME            - The radius location name.
#USER_NAME                      - Username used to login.
#FRAMED_IP_ADDRESS              - The client's IP address.
#CALLING_STATION_ID             - The client's MAC address.
#CALLED_STATION_ID              - The MAC address of the hotspot interface.
#SESSION_TIMEOUT                - The max session time in sec, as set locally or by RADIUS Session-Timeout.
#                                   (0, meaning unlimited).
#IDLE_TIMEOUT                   - The max idle time, as set locally or by RADIUS Idle-Timeout (0, meaning unlimited).
#COOVACHILLI_MAX_INPUT_OCTETS   - Max input octets set locally or by RADIUS ChilliSpot-Max-Input-Octets.
#COOVACHILLI_MAX_OUTPUT_OCTETS  - Max output octets set locally or by RADIUS ChilliSpot-Max-Output-Octets.
#COOVACHILLI_MAX_TOTAL_OCTETS   - Max total octets set by RADIUS ChilliSpot-Max-Total-Octets.
#INPUT_OCTETS                   - Number of octets received during the session.
#OUTPUT_OCTETS                  - Number of octets transmitted during the session.
#INPUT_PACKETS                  - Number of packets received during the session.
#OUTPUT_PACKETS                 - Number of packets transmitted during the session.
#SESSION_TIME                   - Session duration in seconds.
#TERMINATE_CAUSE                - 1=User-Request, 2=Lost-Carrier, 4=Idle-Timeout, 5=Session-Timeout, 11=NAS-Reboot
#IDLE_TIME                      - Idle time duration in seconds.
#ACCT_SESSION_ID                - Unique Accounting ID
#ACCT_INTERIM_INTERVAL          - Interim-interval for RADIUS accounting unless otherwise set by RADIUS
#                                   (defaults to 0, meaning unlimited).
#WISPR_LOCATION_ID              - Radius location id
#WISPR_LOCATION_NAME            - Radius location name
