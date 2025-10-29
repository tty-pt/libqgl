#include "../include/ttypt/qgl.h"

#include <stddef.h>

#include <ttypt/qsys.h>

#include <GLFW/glfw3.h>

typedef struct {
	qgl_key_cb_t *cb;
} input_t;

static input_t cbs[GLFW_KEY_LAST + 1];
unsigned keys[GLFW_KEY_LAST + 1];

qgl_key_cb_t *qgl_key_default;

void
input_call(unsigned short code,
		unsigned short value,
		int type)
{
	input_t *inp = &cbs[code];
	keys[code] = value;
	/* WARN("key: %hu %hu %d\n", code, value, type); */

	if ((qgl_key_default && qgl_key_default(code, value, type)) || !inp->cb)
		return;

	inp->cb(code, value, type);
}

void
qgl_key_reg(unsigned short key, qgl_key_cb_t *cb)
{
	cbs[key].cb = cb;
}

void
qgl_key_default_reg(qgl_key_cb_t *cb)
{
	qgl_key_default = cb;
}

unsigned short
qgl_key_val(unsigned short code)
{
	return keys[code];
}
