module("luci.controller.mosquitto", package.seeall)

function index()
  entry( { "admin", "services", "mqtt"}, alias ("admin", "services", "mqtt", "mqtt"), _("MQTT"), 11)
  entry( { "admin", "services", "mqtt", "mqtt" }, cbi("mqtt_broker"), _("Broker"), 1).leaf = true
  entry( { "admin", "services", "mqtt", "publisher" }, cbi("mqtt_pub"), _("Publisher"), 2).leaf = true
end
