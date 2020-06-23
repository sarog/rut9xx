cbimap = Map("asterisk", "asterisk", "")

module = cbimap:section(TypedSection, "module", translate("Modules"), "")
module.anonymous = true

app_alarmreceiver = module:option(ListValue, "app_alarmreceiver", translate("Alarm Receiver Application"), "")
app_alarmreceiver:value("yes", "Load"))
app_alarmreceiver:value("no", "Do Not Load"))
app_alarmreceiver:value("auto", "Load as Required"))
app_alarmreceiver.rmempty = true

app_authenticate = module:option(ListValue, "app_authenticate", translate("Authentication Application"), "")
app_authenticate:value("yes", translate("Load"))
app_authenticate:value("no", translate("Do Not Load"))
app_authenticate:value("auto", translate("Load as Required"))
app_authenticate.rmempty = true

app_cdr = module:option(ListValue, "app_cdr", translate("Make sure asterisk doesnt save CDR"), "")
app_cdr:value("yes", translate("Load"))
app_cdr:value("no", translate("Do Not Load"))
app_cdr:value("auto", translate("Load as Required"))
app_cdr.rmempty = true

app_chanisavail = module:option(ListValue, "app_chanisavail", translate("Check if channel is available"), "")
app_chanisavail:value("yes", translate("Load"))
app_chanisavail:value("no", translate("Do Not Load"))
app_chanisavail:value("auto", translate("Load as Required"))
app_chanisavail.rmempty = true

app_chanspy = module:option(ListValue, "app_chanspy", translate("Listen in on any channel"), "")
app_chanspy:value("yes", translate("Load"))
app_chanspy:value("no", translate("Do Not Load"))
app_chanspy:value("auto", translate("Load as Required"))
app_chanspy.rmempty = true

app_controlplayback = module:option(ListValue, "app_controlplayback", translate("Control Playback Application"), "")
app_controlplayback:value("yes", translate("Load"))
app_controlplayback:value("no", translate("Do Not Load"))
app_controlplayback:value("auto", translate("Load as Required"))
app_controlplayback.rmempty = true

app_cut = module:option(ListValue, "app_cut", translate("Cuts up variables"), "")
app_cut:value("yes", translate("Load"))
app_cut:value("no", translate("Do Not Load"))
app_cut:value("auto", translate("Load as Required"))
app_cut.rmempty = true

app_db = module:option(ListValue, "app_db", translate("Database access functions"), "")
app_db:value("yes", translate("Load"))
app_db:value("no", translate("Do Not Load"))
app_db:value("auto", translate("Load as Required"))
app_db.rmempty = true

app_dial = module:option(ListValue, "app_dial", translate("Dialing Application"), "")
app_dial:value("yes", translate("Load"))
app_dial:value("no", translate("Do Not Load"))
app_dial:value("auto", translate("Load as Required"))
app_dial.rmempty = true

app_dictate = module:option(ListValue, "app_dictate", translate("Virtual Dictation Machine Application"), "")
app_dictate:value("yes", translate("Load"))
app_dictate:value("no", translate("Do Not Load"))
app_dictate:value("auto", translate("Load as Required"))
app_dictate.rmempty = true

app_directed_pickup = module:option(ListValue, "app_directed_pickup", translate("Directed Call Pickup Support"), "")
app_directed_pickup:value("yes", translate("Load"))
app_directed_pickup:value("no", translate("Do Not Load"))
app_directed_pickup:value("auto", translate("Load as Required"))
app_directed_pickup.rmempty = true

app_directory = module:option(ListValue, "app_directory", translate("Extension Directory"), "")
app_directory:value("yes", translate("Load"))
app_directory:value("no", translate("Do Not Load"))
app_directory:value("auto", translate("Load as Required"))
app_directory.rmempty = true

app_disa = module:option(ListValue, "app_disa", translate("DISA (Direct Inward System Access) Application"), "")
app_disa:value("yes", translate("Load"))
app_disa:value("no", translate("Do Not Load"))
app_disa:value("auto", translate("Load as Required"))
app_disa.rmempty = true

app_dumpchan = module:option(ListValue, "app_dumpchan", translate("Dump channel variables Application"), "")
app_dumpchan:value("yes", translate("Load"))
app_dumpchan:value("no", translate("Do Not Load"))
app_dumpchan:value("auto", translate("Load as Required"))
app_dumpchan.rmempty = true

app_echo = module:option(ListValue, "app_echo", translate("Simple Echo Application"), "")
app_echo:value("yes", translate("Load"))
app_echo:value("no", translate("Do Not Load"))
app_echo:value("auto", translate("Load as Required"))
app_echo.rmempty = true

app_enumlookup = module:option(ListValue, "app_enumlookup", translate("ENUM Lookup"), "")
app_enumlookup:value("yes", translate("Load"))
app_enumlookup:value("no", translate("Do Not Load"))
app_enumlookup:value("auto", translate("Load as Required"))
app_enumlookup.rmempty = true

app_eval = module:option(ListValue, "app_eval", translate("Reevaluates strings"), "")
app_eval:value("yes", translate("Load"))
app_eval:value("no", translate("Do Not Load"))
app_eval:value("auto", translate("Load as Required"))
app_eval.rmempty = true

app_exec = module:option(ListValue, "app_exec", translate("Executes applications"), "")
app_exec:value("yes", translate("Load"))
app_exec:value("no", translate("Do Not Load"))
app_exec:value("auto", translate("Load as Required"))
app_exec.rmempty = true

app_externalivr = module:option(ListValue, "app_externalivr", translate("External IVR application interface"), "")
app_externalivr:value("yes", translate("Load"))
app_externalivr:value("no", translate("Do Not Load"))
app_externalivr:value("auto", translate("Load as Required"))
app_externalivr.rmempty = true

app_forkcdr = module:option(ListValue, "app_forkcdr", translate("Fork The CDR into 2 seperate entities"), "")
app_forkcdr:value("yes", translate("Load"))
app_forkcdr:value("no", translate("Do Not Load"))
app_forkcdr:value("auto", translate("Load as Required"))
app_forkcdr.rmempty = true

app_getcpeid = module:option(ListValue, "app_getcpeid", translate("Get ADSI CPE ID"), "")
app_getcpeid:value("yes", translate("Load"))
app_getcpeid:value("no", translate("Do Not Load"))
app_getcpeid:value("auto", translate("Load as Required"))
app_getcpeid.rmempty = true

app_groupcount = module:option(ListValue, "app_groupcount", translate("Group Management Routines"), "")
app_groupcount:value("yes", translate("Load"))
app_groupcount:value("no", translate("Do Not Load"))
app_groupcount:value("auto", translate("Load as Required"))
app_groupcount.rmempty = true

app_ices = module:option(ListValue, "app_ices", translate("Encode and Stream via icecast and ices"), "")
app_ices:value("yes", translate("Load"))
app_ices:value("no", translate("Do Not Load"))
app_ices:value("auto", translate("Load as Required"))
app_ices.rmempty = true

app_image = module:option(ListValue, "app_image", translate("Image Transmission Application"), "")
app_image:value("yes", translate("Load"))
app_image:value("no", translate("Do Not Load"))
app_image:value("auto", translate("Load as Required"))
app_image.rmempty = true

app_lookupblacklist = module:option(ListValue, "app_lookupblacklist", translate("Look up Caller*ID name/number from black"), "")
app_lookupblacklist:value("yes", translate("Load"))
app_lookupblacklist:value("no", translate("Do Not Load"))
app_lookupblacklist:value("auto", translate("Load as Required"))
app_lookupblacklist.rmempty = true

app_lookupcidname = module:option(ListValue, "app_lookupcidname", translate("Look up CallerID Name from local database"), "")
app_lookupcidname:value("yes", translate("Load"))
app_lookupcidname:value("no", translate("Do Not Load"))
app_lookupcidname:value("auto", translate("Load as Required"))
app_lookupcidname.rmempty = true

app_macro = module:option(ListValue, "app_macro", translate("Extension Macros"), "")
app_macro:value("yes", translate("Load"))
app_macro:value("no", translate("Do Not Load"))
app_macro:value("auto", translate("Load as Required"))
app_macro.rmempty = true

app_math = module:option(ListValue, "app_math", translate("A simple math Application"), "")
app_math:value("yes", translate("Load"))
app_math:value("no", translate("Do Not Load"))
app_math:value("auto", translate("Load as Required"))
app_math.rmempty = true

app_md5 = module:option(ListValue, "app_md5", translate("MD5 checksum Application"), "")
app_md5:value("yes", translate("Load"))
app_md5:value("no", translate("Do Not Load"))
app_md5:value("auto", translate("Load as Required"))
app_md5.rmempty = true

app_milliwatt = module:option(ListValue, "app_milliwatt", translate("Digital Milliwatt (mu-law) Test Application"), "")
app_milliwatt:value("yes", translate("Load"))
app_milliwatt:value("no", translate("Do Not Load"))
app_milliwatt:value("auto", translate("Load as Required"))
app_milliwatt.rmempty = true

app_mixmonitor = module:option(ListValue, "app_mixmonitor", translate("Record a call and mix the audio during the recording"), "")
app_mixmonitor:value("yes", translate("Load"))
app_mixmonitor:value("no", translate("Do Not Load"))
app_mixmonitor:value("auto", translate("Load as Required"))
app_mixmonitor.rmempty = true

app_parkandannounce = module:option(ListValue, "app_parkandannounce", translate("Call Parking and Announce Application"), "")
app_parkandannounce:value("yes", translate("Load"))
app_parkandannounce:value("no", translate("Do Not Load"))
app_parkandannounce:value("auto", translate("Load as Required"))
app_parkandannounce.rmempty = true

app_playback = module:option(ListValue, "app_playback", translate("Trivial Playback Application"), "")
app_playback:value("yes", translate("Load"))
app_playback:value("no", translate("Do Not Load"))
app_playback:value("auto", translate("Load as Required"))
app_playback.rmempty = true

app_privacy = module:option(ListValue, "app_privacy", translate("Require phone number to be entered"), "")
app_privacy:value("yes", translate("Load"))
app_privacy:value("no", translate("Do Not Load"))
app_privacy:value("auto", translate("Load as Required"))
app_privacy.rmempty = true

app_queue = module:option(ListValue, "app_queue", translate("True Call Queueing"), "")
app_queue:value("yes", translate("Load"))
app_queue:value("no", translate("Do Not Load"))
app_queue:value("auto", translate("Load as Required"))
app_queue.rmempty = true

app_random = module:option(ListValue, "app_random", translate("Random goto"), "")
app_random:value("yes", translate("Load"))
app_random:value("no", translate("Do Not Load"))
app_random:value("auto", translate("Load as Required"))
app_random.rmempty = true

app_read = module:option(ListValue, "app_read", translate("Read Variable Application"), "")
app_read:value("yes", translate("Load"))
app_read:value("no", translate("Do Not Load"))
app_read:value("auto", translate("Load as Required"))
app_read.rmempty = true

app_readfile = module:option(ListValue, "app_readfile", translate("Read in a file"), "")
app_readfile:value("yes", translate("Load"))
app_readfile:value("no", translate("Do Not Load"))
app_readfile:value("auto", translate("Load as Required"))
app_readfile.rmempty = true

app_realtime = module:option(ListValue, "app_realtime", translate("Realtime Data Lookup/Rewrite"), "")
app_realtime:value("yes", "translate(Load"))
app_realtime:value("no", translate("Do Not Load"))
app_realtime:value("auto", translate("Load as Required"))
app_realtime.rmempty = true

app_record = module:option(ListValue, "app_record", translate("Trivial Record Application"), "")
app_record:value("yes", translate("Load"))
app_record:value("no", translate("Do Not Load"))
app_record:value("auto", translate("Load as Required"))
app_record.rmempty = true

app_sayunixtime = module:option(ListValue, "app_sayunixtime", translate("Say time"), "")
app_sayunixtime:value("yes", translate("Load"))
app_sayunixtime:value("no", translate("Do Not Load"))
app_sayunixtime:value("auto", translate("Load as Required"))
app_sayunixtime.rmempty = true

app_senddtmf = module:option(ListValue, "app_senddtmf", translate("Send DTMF digits Application"), "")
app_senddtmf:value("yes", translate("Load"))
app_senddtmf:value("no", translate("Do Not Load"))
app_senddtmf:value("auto", translate("Load as Required"))
app_senddtmf.rmempty = true

app_sendtext = module:option(ListValue, "app_sendtext", translate("Send Text Applications"), "")
app_sendtext:value("yes", translate("Load"))
app_sendtext:value("no", translate("Do Not Load"))
app_sendtext:value("auto", translate("Load as Required"))
app_sendtext.rmempty = true

app_setcallerid = module:option(ListValue, "app_setcallerid", translate("Set CallerID Application"), "")
app_setcallerid:value("yes", translate("Load"))
app_setcallerid:value("no", translate("Do Not Load"))
app_setcallerid:value("auto", translate("Load as Required"))
app_setcallerid.rmempty = true

app_setcdruserfield = module:option(ListValue, "app_setcdruserfield", translate("CDR user field apps"), "")
app_setcdruserfield:value("yes", translate("Load"))
app_setcdruserfield:value("no", translate("Do Not Load"))
app_setcdruserfield:value("auto", translate("Load as Required"))
app_setcdruserfield.rmempty = true

app_setcidname = module:option(ListValue, "app_setcidname", translate("load => .so ; Set CallerID Name"), "")
app_setcidname:value("yes", translate("Load"))
app_setcidname:value("no", translate("Do Not Load"))
app_setcidname:value("auto", translate("Load as Required"))
app_setcidname.rmempty = true

app_setcidnum = module:option(ListValue, "app_setcidnum", translate("load => .so ; Set CallerID Number"), "")
app_setcidnum:value("yes", translate("Load"))
app_setcidnum:value("no", translate("Do Not Load"))
app_setcidnum:value("auto", translate("Load as Required"))
app_setcidnum.rmempty = true

app_setrdnis = module:option(ListValue, "app_setrdnis", translate("Set RDNIS Number"), "")
app_setrdnis:value("yes", translate("Load"))
app_setrdnis:value("no", translate("Do Not Load"))
app_setrdnis:value("auto", translate("Load as Required"))
app_setrdnis.rmempty = true

app_settransfercapability = module:option(ListValue, "app_settransfercapability", translate("Set ISDN Transfer Capability"), "")
app_settransfercapability:value("yes", translate("Load"))
app_settransfercapability:value("no", translate("Do Not Load"))
app_settransfercapability:value("auto", translate("Load as Required"))
app_settransfercapability.rmempty = true

app_sms = module:option(ListValue, "app_sms", translate("SMS/PSTN handler"), "")
app_sms:value("yes", translate("Load"))
app_sms:value("no", translate("Do Not Load"))
app_sms:value("auto", translate("Load as Required"))
app_sms.rmempty = true

app_softhangup = module:option(ListValue, "app_softhangup", translate("Hangs up the requested channel"), "")
app_softhangup:value("yes", translate("Load"))
app_softhangup:value("no", translate("Do Not Load"))
app_softhangup:value("auto", translate("Load as Required"))
app_softhangup.rmempty = true

app_stack = module:option(ListValue, "app_stack", translate("Stack Routines"), "")
app_stack:value("yes", translate("Load"))
app_stack:value("no", translate("Do Not Load"))
app_stack:value("auto", translate("Load as Required"))
app_stack.rmempty = true

app_system = module:option(ListValue, "app_system", translate("Generic System() application"), "")
app_system:value("yes", translate("Load"))
app_system:value("no", translate("Do Not Load"))
app_system:value("auto", translate("Load as Required"))
app_system.rmempty = true

app_talkdetect = module:option(ListValue, "app_talkdetect", translate("Playback with Talk Detection"), "")
app_talkdetect:value("yes", translate("Load"))
app_talkdetect:value("no", translate("Do Not Load"))
app_talkdetect:value("auto", translate("Load as Required"))
app_talkdetect.rmempty = true

app_test = module:option(ListValue, "app_test", translate("Interface Test Application"), "")
app_test:value("yes", translate("Load"))
app_test:value("no", translate("Do Not Load"))
app_test:value("auto", translate("Load as Required"))
app_test.rmempty = true

app_transfer = module:option(ListValue, "app_transfer", translate("Transfer"), "")
app_transfer:value("yes", translate("Load"))
app_transfer:value("no", translate("Do Not Load"))
app_transfer:value("auto", translate("Load as Required"))
app_transfer.rmempty = true

app_txtcidname = module:option(ListValue, "app_txtcidname", translate("TXTCIDName"), "")
app_txtcidname:value("yes", translate("Load"))
app_txtcidname:value("no", translate("Do Not Load"))
app_txtcidname:value("auto", translate("Load as Required"))
app_txtcidname.rmempty = true

app_url = module:option(ListValue, "app_url", translate("Send URL Applications"), "")
app_url:value("yes", translate("Load"))
app_url:value("no", translate("Do Not Load"))
app_url:value("auto", translate("Load as Required"))
app_url.rmempty = true

app_userevent = module:option(ListValue, "app_userevent", translate("Custom User Event Application"), "")
app_userevent:value("yes", translate("Load"))
app_userevent:value("no", translate("Do Not Load"))
app_userevent:value("auto", translate("Load as Required"))
app_userevent.rmempty = true

app_verbose = module:option(ListValue, "app_verbose", translate("Send verbose output"), "")
app_verbose:value("yes", translate("Load"))
app_verbose:value("no", translate("Do Not Load"))
app_verbose:value("auto", translate("Load as Required"))
app_verbose.rmempty = true

app_voicemail = module:option(ListValue, "app_voicemail", translate("Voicemail"), "")
app_voicemail:value("yes", translate("Load"))
app_voicemail:value("no", translate("Do Not Load"))
app_voicemail:value("auto", translate("Load as Required"))
app_voicemail.rmempty = true

app_waitforring = module:option(ListValue, "app_waitforring", translate("Waits until first ring after time"), "")
app_waitforring:value("yes", translate("Load"))
app_waitforring:value("no", translate("Do Not Load"))
app_waitforring:value("auto", translate("Load as Required"))
app_waitforring.rmempty = true

app_waitforsilence = module:option(ListValue, "app_waitforsilence", translate("Wait For Silence Application"), "")
app_waitforsilence:value("yes", translate("Load"))
app_waitforsilence:value("no", translate("Do Not Load"))
app_waitforsilence:value("auto", translate("Load as Required"))
app_waitforsilence.rmempty = true

app_while = module:option(ListValue, "app_while", translate("While Loops and Conditional Execution"), "")
app_while:value("yes", translate("Load"))
app_while:value("no", translate("Do Not Load"))
app_while:value("auto", translate("Load as Required"))
app_while.rmempty = true


return cbimap
