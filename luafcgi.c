/*
 * Copyright (c) 2013, Micro Systems Marc Balmer, CH-5073 Gipf-Oberfrick
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Micro Systems Marc Balmer nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* FastCGI interface for Lua */

#include <fcgiapp.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdlib.h>
#include <string.h>

static FCGX_Stream *in, *out, *err;
static FCGX_ParamArray env;

static int
fcgi_accept(lua_State *L)
{
	lua_pushinteger(L, FCGX_Accept(&in, &out, &err, &env));
	return 1;
}

static int
fcgi_finish(lua_State *L)
{
	FCGX_Finish();
	return 0;
}

static int
fcgi_flush(lua_State *L)
{
	FCGX_FFlush(out);
	return 0;
}

static int
fcgi_get_line(lua_State *L)
{
	int len;
	char *str, *p;

	len = luaL_checkinteger(L, 1);
	str = malloc(len + 1);
	if (str == NULL)
		return luaL_error(L, "memory error");
	p = FCGX_GetLine(str, len, in);
	if (p == NULL)
		lua_pushnil(L);
	else
		lua_pushstring(L, str);
	free(str);
	return 1;
}

static int
fcgi_get_str(lua_State *L)
{
	int len, rc;
	char *str;

	len = luaL_checkinteger(L, 1);
	str = malloc(len + 1);
	if (str == NULL)
		return luaL_error(L, "memory error");
	rc = FCGX_GetStr(str, len, in);
	if (rc == 0)
		lua_pushnil(L);
	else
		lua_pushstring(L, str);
	lua_pushinteger(L, rc);
	free(str);
	return 2;
}

static int
fcgi_get_param(lua_State *L)
{
	char *p;

	p = FCGX_GetParam(luaL_checkstring(L, -1), env);
	if (p == NULL)
		lua_pushnil(L);
	else
		lua_pushstring(L, p);
	return 1;
}


static int
fcgi_get_env(lua_State *L)
{
	char **p = env;
  char *v = NULL;

  if (p == NULL) {
		lua_pushnil(L);
    return 1;
  }

  lua_newtable(L);
  while (*p) {
    v = strchr(*p, '=');
    if (v) {
      lua_pushlstring(L, *p, v-*p);
      lua_pushstring(L, ++v);
      lua_settable(L, -3);
    }
    p++;
  }
  return 1;
}

static int
fcgi_open_socket(lua_State *L)
{
	int sock;

	sock = FCGX_OpenSocket(luaL_checkstring(L, 1), luaL_checkinteger(L, 2));
	lua_pushinteger(L, sock);
	return 1;
}

static int
fcgi_put_str(lua_State *L)
{
	const char *str;

	str = luaL_checkstring(L, 1);
	lua_pushinteger(L, FCGX_PutStr(str, strlen(str), out));
	return 1;
}

static void
fcgi_set_info(lua_State *L)
{
	lua_pushliteral(L, "_COPYRIGHT");
	lua_pushliteral(L, "Copyright (C) 2013 micro systems marc balmer");
	lua_settable(L, -3);
	lua_pushliteral(L, "_DESCRIPTION");
	lua_pushliteral(L, "FastCGI interface for Lua");
	lua_settable(L, -3);
	lua_pushliteral(L, "_VERSION");
	lua_pushliteral(L, "fcgi 1.0.0");
	lua_settable(L, -3);
}

int
luaopen_fcgi(lua_State* L)
{
	static const struct luaL_Reg methods[] = {
		{ "accept",	fcgi_accept },
		{ "finish",	fcgi_finish },
		{ "flush",	fcgi_flush },
		{ "getLine",	fcgi_get_line },
		{ "getStr",	fcgi_get_str },
		{ "getParam",	fcgi_get_param },
		{ "getEnv",	fcgi_get_env},
		{ "openSocket",	fcgi_open_socket },
		{ "putStr",	fcgi_put_str },
		{ NULL,		NULL }
	};

	luaL_register(L, "fcgi", methods);
	fcgi_set_info(L);

	return 1;
}
