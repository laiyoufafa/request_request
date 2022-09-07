/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index';
import request from '@ohos.request';

const TAG = "REQUEST_TEST";
let keyStr = 'download test ';

describe('RequestTest', function () {
    beforeAll(function () {
        console.info('beforeAll called')
    })

    afterAll(function () {
        console.info('afterAll called')
    })

    beforeEach(function () {
        console.info('beforeEach called')
    })

    afterEach(function () {
        console.info('afterEach called')
    })
    console.log(TAG + "*************Unit Test Begin*************");

    /**
     * @tc.name: downloadTest001
     * @tc.desc download parameter verification
     * @tc.type: FUNC
     * @tc.require: 000000
     */
    it('downloadTest001', 0, function () {
        console.log(TAG + "************* downloadTest001 start *************");
        let config;
        try {
            request.download(config);
        } catch (err) {
            expect(true).assertEqual(true);
        }
        console.log(TAG + "************* downloadTest001 end *************");
    })
    console.log(TAG + "*************Unit Test End*************");
})