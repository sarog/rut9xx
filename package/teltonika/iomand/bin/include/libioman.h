#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <libubus.h>

typedef enum {
	LIOMAN_STATUS_OK,
	LIOMAN_STATUS_ERROR,
	LIOMAN_STATUS_ERROR_IO_NOT_FOUND,
	LIOMAN_STATUS_ERROR_IOMAN,
	LIOMAN_STATUS_ERROR_UBUS,
	LIOMAN_STATUS_ERROR_NOMEM,
	LIOMAN_STATUS_ERROR_INPUT,
	LIOMAN_STATUS_ERROR_UNSUPPORTED,
	LIOMAN_STATUS_ERROR_HW,
	LIOMAN_STATUS_ERROR_UCI,
	_LIOMAN_STATUS_SIZE,
} lioman_status;

typedef enum {
	LIOMAN_TYPE_UNKNOWN,
	LIOMAN_TYPE_DIO,
	LIOMAN_TYPE_RELAY,
	LIOMAN_TYPE_ADC,
	LIOMAN_TYPE_ACL,
	LIOMAN_TYPE_THERMAL,
	LIOMAN_TYPE_DWI,
} lioman_io_type;

typedef enum {
	LIOMAN_FILTER_FOREACH = 0,
	LIOMAN_FILTER_ACCEPT  = 1 << 0,
	LIOMAN_FILTER_REJECT  = 1 << 1,
	LIOMAN_FILTER_STOP    = 1 << 2,
} lioman_filter_flag;

struct lioman_io_meta {
	uint32_t ubus_obj_id;
	char name[16];
	lioman_io_type type;
};

struct lioman_location {
	char block_type[16];
	uint8_t block_id;
	uint8_t npins;
	uint8_t pins[8];
	char io_name[32];
	char io_param[4];
};

struct lioman_adcc { //custom analog input
	char cname[16];
	char cunit[16];
	float cvalue;
	float cmul;
	float coff;
	float cadd;
	float cdiv;
};

struct lioman_io {
	struct lioman_io_meta meta; // set by lioman_get_*
	struct lioman_location location;
	bool save_conf; // whether to save state changes via lioman_set* functions to the iomand conf

	union {
		struct {
			bool high : 1; // must remain the 1st
			bool in : 1; // in or out
			bool inverted_input : 1;
			bool bidirectional : 1;
		} dio;

		struct {
			bool closed : 1; // must remain the 1st
			bool latching : 1; // latching or basic
		} relay;

		struct {
			struct lioman_adcc adcc;
			float value;
		} adc;

		struct {
			float value;
		} therm;

		struct {
			struct lioman_adcc adcc;
			float raw_value;
			float current;
			float percentage;
			bool active : 1; // active or inactive
		} acl;

		struct {
			bool high : 1; // must remain the 1st
			bool dry : 1; // dry or wet
			bool inverted_input : 1;
		} dwi;
	};

	union {
		void *p;
		uintptr_t u; // same width as void*
		int d;
		bool b;
	} user_data; // libioman does nothing with this
};

struct lioman_io_array {
	unsigned len;
	struct lioman_io ios[];
};

struct lioman_description {
	const char *type;
	const char *state;
};

typedef void (*lioman_cb)(const struct lioman_io *, void *user_data);
typedef lioman_filter_flag (*lioman_filter_cb_t)(const struct lioman_io *, void *user_data);

#define LIOMAN_IS_INPUT(io)                                                                                                                          \
	(((io).meta.type == LIOMAN_TYPE_DIO && ((io).dio.in || (io).dio.bidirectional)) || (io).meta.type == LIOMAN_TYPE_DWI ||                      \
	 (io).meta.type == LIOMAN_TYPE_ADC || (io).meta.type == LIOMAN_TYPE_ACL)

#define LIOMAN_IS_CURRENTLY_INPUT(io)                                          \
	((io).meta.type == LIOMAN_TYPE_DIO ? (io).dio.in : LIOMAN_IS_INPUT(io))

#define lioman_foreach(lfe__array, lfe__elm, lfe__scope)                                                                                             \
	do {                                                                                                                                         \
		struct lioman_io *lfe__elm = lfe__array->ios;                                                                                        \
		for (unsigned liaf__c = lfe__array->len; liaf__c; liaf__c--, lfe__elm++) {                                                           \
			lfe__scope                                                                                                                   \
		}                                                                                                                                    \
	} while (0)

#define lioman_foreach_const(lfec__array, lfec__elm, lfec__scope)                                                                                    \
	do {                                                                                                                                         \
		const struct lioman_io *lfec__elm = lfec__array->ios;                                                                                \
		for (unsigned lciaf__c = lfec__array->len; lciaf__c; lciaf__c--, lfec__elm++)                                                        \
			lfec__scope                                                                                                                  \
	} while (0)

/**
  * Describes an error status
  * @param[in] status a status other than LIOMAN_STATUS_OK
  * @return static memory, not to be free()d! string containing the error description
  */
const char *lioman_strerror(lioman_status status);

/**
  * Describes an error status
  * @param[in] source a pointer to an IO struct previously retrieved by a getter function
  * @return struct containing strings describing the IO
  */
struct lioman_description lioman_describe(const struct lioman_io *source);

///////////////////////////////getters////////////////////////////////////////

/**
  * Retrieves a single IO by name
  * @param[in] ctx Ubus context
  * @param[in] name the IO to retrieve, the part after ioman.gpio.
  * @param[out] dest pointer to an allocated struct where IO info will be written
  */
lioman_status lioman_get_by_name(struct ubus_context *ctx, const char *name, struct lioman_io *dest);

/**
  * Retrieves a single IO by ubus id
  * @param[in] ctx Ubus context
  * @param[in] ubus id of the IO to get, retrieved by ubus_lookup_id() for example
  * @param[out] dest pointer to an already allocated struct where IO info will be written
  */
lioman_status lioman_get_by_id(struct ubus_context *ctx, uint32_t id, struct lioman_io *dest);

/**
  * Retrieves all available IOs
  * @param[in] ctx Ubus context
  * @param[out] array must be freed()! pointer to a struct pointer, which holds a variable length array of struct lioman_io
  */
lioman_status lioman_get_all(struct ubus_context *ctx, struct lioman_io_array **array);

/**
  * Retrieves each IO that the filter callback accepts, can also be used as a quick foreach of all the available IOs, if array == NULL
  * @param[in] ctx Ubus context
  * @param[out] array must be freed()! pointer to a struct pointer, which holds a variable length array of struct lioman_io
  * @param[in] filter callback that either rejects or accepts an IO into the array. Gets called twice if array != NULL - callback is expected to return the same values on both calls. Gets called once if array == NULL - to be used as a quick foreach of all avalable IOs.
  */
lioman_status lioman_get_filtered(struct ubus_context *ctx, struct lioman_io_array **dest, lioman_filter_cb_t filter, void *user_data);

// all of the following functions are to be used on `struct lioman_io *`s that have been acquired with any of the lioman_get_* functions

/**
  * Updates value fields of an IO struct previously retrieved by any of the lioman_get_* functions, can be called any number of times on the same IO struct
  * @param[in] ctx Ubus context
  * @param[out] target pointer to an IO struct previously retrieved by a getter function
  */
lioman_status lioman_update(struct ubus_context *ctx, struct lioman_io *target);

/**
  * Updates value fields in an IO array previously retrieved by lioman_get_{all,filtered} functions, can be called any number of times on the same IO struct array
  * @param[in] ctx Ubus context
  * @param[out] array pointer to an IO array struct, previously retrieved by a getter function
  */
lioman_status lioman_update_array(struct ubus_context *ctx, struct lioman_io_array *array);

///////////////////////////////setters////////////////////////////////////////

/**
  * Updates an IO from a modified IO struct previously retrieved by a getter function, can be called any number of times on the same IO struct
  * @param[in] ctx Ubus context
  * @param[in] target pointer to an IO struct
  */
lioman_status lioman_set(struct ubus_context *ctx, const struct lioman_io *target);

/**
  * Updates IOs from a modified IO array previously retrieved by lioman_get_{all,filtered} functions, can be called any number of times on the same IO struct array
  * @param[in] ctx Ubus context
  * @param[in] array pointer to an IO array struct
  */
lioman_status lioman_set_array(struct ubus_context *ctx, const struct lioman_io_array *array);

/**
  * Inverts an Output - if a Digital Output is Low, sets it High; if a Relay is Closed, Opens it; doesn't update target with the new state, merely inverts
  * @param[in] ctx Ubus context
  * @param[in] target pointer to an IO struct
  */
lioman_status lioman_invert(struct ubus_context *ctx, const struct lioman_io *target);

/**
  * Subscribes to IO state changes; currently, iomand only notifies on Input state changes. A subsequent call on the same IO without first unsubscribing replaces the callback function
  * @param[in] ctx Ubus context
  * @param[in] target pointer to an IO struct of an IO to subscribe to, previously retrieved by a getter function; only accessed while subscribing and never again after function returns
  * @param[in] cb called on IO state change, gets passed a pointer to a read-only IO struct with the new state - static memory so not to be free()d, also not to be used outside the callback scope
  * @param[in] user_data gets passed to the callback. Pass NULL if unsure
  */
lioman_status lioman_subscribe(struct ubus_context *ctx, const struct lioman_io *target, lioman_cb cb, void *user_data);

/**
  * Subscribes to IO state changes; currently, iomand only notifies on Input state changes; a subsequent call on the same IO without first unsubscribing replaces the callback function
  * @param[in] ctx Ubus context
  * @param[in] ubus_obj_id ubus id of the IO to subscribe to, retrieved by ubus_lookup_id() for example
  * @param[in] cb called on IO state change, gets passed a pointer to a read-only IO struct with the new state - static memory so not to be free()d, also not to be used outside the callback scope
  * @param[in] user_data gets passed to the callback. Pass NULL if unsure
  */
lioman_status lioman_subscribe_by_id(struct ubus_context *ctx, uint32_t ubus_obj_id, lioman_cb cb, void *user_data);

/**
  * Subscribes to IO state changes; currently, iomand only notifies on Input state changes; a subsequent call on the same IO without first unsubscribing replaces the callback function
  * @param[in] ctx Ubus context
  * @param[in] name the IO to subscribe to, the part after ioman.gpio.
  * @param[in] cb called on IO state change, gets passed a pointer to a read-only IO struct with the new state - static memory so not to be free()d, also not to be used outside the callback scope
  * @param[in] user_data gets passed to the callback. Pass NULL if unsure
  */
lioman_status lioman_subscribe_by_name(struct ubus_context *ctx, const char *name, lioman_cb cb, void *user_data);

/**
  * Unsubscribes from IO state changes
  * @param[in] ctx Ubus context
  * @param[in] target pointer to an IO struct previously retrieved by a getter function; doesn't have to be the same pointer as was used to subscribe
  */
lioman_status lioman_unsubscribe(struct ubus_context *ctx, struct lioman_io *target);

/**
  * Unsubscribes from IO state changes
  * @param[in] ctx Ubus context
  * @param[in] ubus_obj_id ubus id of the IO to unsubscribe from, retrieved by ubus_lookup_id() for example
  */
lioman_status lioman_unsubscribe_by_id(struct ubus_context *ctx, uint32_t ubus_obj_id);

/**
  * Unsubscribes from IO state changes
  * @param[in] ctx Ubus context
  * @param[in] name the IO to unsubscribe from, the part after ioman.gpio.
  */
lioman_status lioman_unsubscribe_by_name(struct ubus_context *ctx, const char *name);
