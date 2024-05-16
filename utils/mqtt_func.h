#ifndef MQTT_FUNC_H_INCLUDED
#define MQTT_FUNC_H_INCLUDED

void on_connect(struct mosquitto *mosq, void *obj, int rc);
void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);

#endif