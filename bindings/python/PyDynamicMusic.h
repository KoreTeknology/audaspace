/*******************************************************************************
* Copyright 2009-2015 Juan Francisco Crespo Gal�n
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
******************************************************************************/

#pragma once

#include <Python.h>
#include "Audaspace.h"

typedef void Reference_DynamicMusic;

typedef struct {
	PyObject_HEAD
		Reference_DynamicMusic* dynamicMusic;
} DynamicMusicP;

extern AUD_API PyObject* DynamicMusic_empty();
extern AUD_API DynamicMusicP* checkDynamicMusic(PyObject* dynamicMusic);

bool initializeDynamicMusic();
void addDynamicMusicToModule(PyObject* module);