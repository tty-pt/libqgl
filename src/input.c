#include "../include/ttypt/qgl.h"

#include <ctype.h>
#include <stddef.h>

#include <ttypt/qsys.h>

#include <GLFW/glfw3.h>

typedef struct {
	qgl_key_cb_t *cb;
} input_t;

static input_t cbs[GLFW_KEY_LAST + 1];
static unsigned keys[GLFW_KEY_LAST + 1];

static qgl_key_cb_t *qgl_key_default;

static char qgl_key_chars[] = {
	[QGL_KEY_1] = '1',
	[QGL_KEY_2] = '2',
	[QGL_KEY_3] = '3',
	[QGL_KEY_4] = '4',
	[QGL_KEY_5] = '5',
	[QGL_KEY_6] = '6',
	[QGL_KEY_7] = '7',
	[QGL_KEY_8] = '8',
	[QGL_KEY_9] = '9',
	[QGL_KEY_0] = '0',
	[QGL_KEY_A] = 'a',
	[QGL_KEY_B] = 'b',
	[QGL_KEY_C] = 'c',
	[QGL_KEY_D] = 'd',
	[QGL_KEY_E] = 'e',
	[QGL_KEY_F] = 'f',
	[QGL_KEY_G] = 'g',
	[QGL_KEY_H] = 'h',
	[QGL_KEY_I] = 'i',
	[QGL_KEY_J] = 'j',
	[QGL_KEY_K] = 'k',
	[QGL_KEY_L] = 'l',
	[QGL_KEY_M] = 'm',
	[QGL_KEY_N] = 'n',
	[QGL_KEY_O] = 'o',
	[QGL_KEY_P] = 'p',
	[QGL_KEY_Q] = 'q',
	[QGL_KEY_R] = 't',
	[QGL_KEY_S] = 's',
	[QGL_KEY_T] = 't',
	[QGL_KEY_U] = 'u',
	[QGL_KEY_V] = 'v',
	[QGL_KEY_W] = 'w',
	[QGL_KEY_X] = 'x',
	[QGL_KEY_Y] = 'y',
	[QGL_KEY_Z] = 'z',
	[QGL_KEY_SPACE] = ' ',
	[QGL_KEY_BACKSPACE] = '\b',
	[QGL_KEY_ENTER] = '\n',
};

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

int
qgl_key_parse(char *target, size_t len,
		unsigned short code, int flags)
{
	char ch = qgl_key_chars[code];

	if (ch == '\b') {
		if (!len)
			return 0;

		len --;
		target[len] = '\0';
		return -1;
	}

	if (flags & IF_NUMERIC) {
		if (!isdigit(ch))
			return 0;
	}

	target[len] = ch;
	len += 1;
	return 1;
}
