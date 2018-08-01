/**
 *  Copyright (c) 2018 La Cool Co SAS
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 *
 */

#include <FS.h>

#include "CoolConfig.h"
#include "CoolLog.h"
#include "CoolAsyncEditor.h"

CoolConfig::CoolConfig(const char *path) : path(path) {};

bool CoolConfig::readFileAsJson() {
  if (!CoolAsyncEditor::getInstance().exist(this->path)) {
    ERROR_VAR("File does not exists:", path);
    return (false);
  }
  this->json = this->buffer.parse(CoolAsyncEditor::getInstance().read(this->path));
  if (!this->json.success()) {
    ERROR_VAR("Failed to parse file as JSON:", this->path);
    return (false);
  }
  DEBUG_VAR("Reading configuration file as JSON:", this->path);
  DEBUG_JSON("Configuration JSON:", this->json);
  return (true);
}

JsonObject &CoolConfig::get() { return this->json; }

void CoolConfig::setConfig(JsonVariant json) { this->json = json; }

bool CoolConfig::writeJsonToFile() {
  if (!CoolAsyncEditor::getInstance().exist(this->path)) {
    ERROR_VAR("Failed to open file for writing:", this->path);
    return (false);
  }
  String tmpJson = CoolAsyncEditor::getInstance().read(this->path);
  json.printTo(tmpJson);
  CoolAsyncEditor::getInstance().write(this->path,tmpJson);
  DEBUG_VAR("JSON config is:", tmpJson);
  DEBUG_VAR("Saved JSON config to:", this->path);
  return (true);
}