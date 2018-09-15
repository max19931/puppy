/*
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CHECKUP_TEST
#define CHECKUP_TEST

#include <EASTL/string.h>

class Test {
    public:
        const char* name() const;

        void test();
    protected:
        Test(const char* name);

        virtual bool setup();

        virtual void run() = 0;

        virtual void teardown();
        ~Test();

    private:
        eastl::string mName;
};

#endif
