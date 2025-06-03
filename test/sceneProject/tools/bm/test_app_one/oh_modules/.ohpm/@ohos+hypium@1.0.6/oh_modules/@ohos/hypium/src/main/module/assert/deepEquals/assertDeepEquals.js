/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

import DeepTypeUtils from './DeepTypeUtils'
function assertDeepEquals(actualValue, expected) {
    console.log('actualValue:' + actualValue + ',expected:' + expected[0]);
    let result = eq(actualValue, expected[0],[], [])
    let msg = logMsg(actualValue, expected[0]);
    return {
        pass: result,
        message: msg
    };
}

function logMsg(actualValue, expected) {
    const aClassName = Object.prototype.toString.call(actualValue);
    const bClassName = Object.prototype.toString.call(expected);
    let actualMsg;
    let expectMsg;
    if(aClassName == "[object Function]") {
        actualMsg = "actualValue Function"
    }else if(aClassName == "[object Promise]") {
        actualMsg = "actualValue Promise"
    }else if(aClassName == "[object Set]" || aClassName == "[object Map]") {
        actualMsg = JSON.stringify(Array.from(actualValue));;
    }else if(aClassName == "[object RegExp]") {
        actualMsg = JSON.stringify(actualValue.source.replace("\\",""));;
    }
    else{
        actualMsg = JSON.stringify(actualValue);
    }
    if(bClassName == "[object Function]") {
        expectMsg = "expected Function"
    }else if(bClassName == "[object Promise]") {
        expectMsg = "expected Promise"
    }else if(aClassName == "[object Set]" || bClassName == "[object Map]") {
        expectMsg = JSON.stringify(Array.from(expected));
    }else if(aClassName == "[object RegExp]") {
        expectMsg = JSON.stringify(expected.source.replace("\\",""));;
    }
    else{
        expectMsg = JSON.stringify(expected);
    }
    return actualMsg + " is not deep equal " + expectMsg;
}

function eq(a, b, aStack, bStack) {
    let result = true;
    console.log('a is:' + a + ',b is:' + b);
    const asymmetricResult = asymmetricMatch_(a,b);
    if (!DeepTypeUtils.isUndefined(asymmetricResult)) {
        return asymmetricResult;
    }

    if (a instanceof Error && b instanceof Error) {
        result = a.message == b.message;
        return result;
    }

    if (a === b) {
        result = a !== 0 || 1 / a == 1 / b;
        return result;
    }

    if (a === null || b === null) {
        result = a === b;
        return result;
    }
    const aClassName = Object.prototype.toString.call(a);
    const bClassName = Object.prototype.toString.call(b);
    console.log('aClassName is:' + aClassName);
    console.log('bClassName is:' + bClassName);
    if (aClassName != bClassName) {
        return false;
    }
    if(aClassName === '[object String]') {
        result = a == String(b);
        return result;
    }
    if(aClassName === '[object Number]') {
        result = a != +a ? b != +b : a === 0 && b === 0 ? 1 / a == 1 / b : a == +b;
        return result;
    }

    if(aClassName === '[object Date]' || aClassName === '[object Boolean]') {
        result = +a == +b;
        return result;
    }
    if(aClassName === '[object ArrayBuffer]') {
        return eq(new Uint8Array(a), new Uint8Array(b), aStack, bStack);
    }

    if(aClassName === '[object RegExp]') {
        return (
            a.source == b.source &&
            a.global == b.global &&
            a.multiline == b.multiline &&
            a.ignoreCase == b.ignoreCase
        );
    }

    if (typeof a != 'object' || typeof b != 'object') {
        return false;
    }

    const aIsDomNode = DeepTypeUtils.isDomNode(a);
    const bIsDomNode = DeepTypeUtils.isDomNode(b);
    if (aIsDomNode && bIsDomNode) {
        result = a.isEqualNode(b);
        return result;
    }
    if (aIsDomNode || bIsDomNode) {
        return false;
    }
    const aIsPromise = DeepTypeUtils.isPromise(a);
    const bIsPromise = DeepTypeUtils.isPromise(b);
    if (aIsPromise && bIsPromise) {
        return a === b;
    }
    let length = aStack.length;
    while (length--) {
        if (aStack[length] == a) {
            return bStack[length] == b;
        }
    }
    aStack.push(a);
    bStack.push(b);
    let size = 0;

    if(aClassName == '[object Array]') {
        const aLength = a.length;
        const bLength = b.length;
        if (aLength !== bLength) {
            return false;
            }
        for (let i = 0; i < aLength || i < bLength; i++) {
            result = eq(i < aLength ? a[i] : void 0, i < bLength ? b[i] : void 0, aStack, bStack) && result;
        }
        if (!result) {
            return false;
        }
    } else if(DeepTypeUtils.isMap(a) && DeepTypeUtils.isMap(b)) {
        if (a.size != b.size) {
            return false;
        }
        const keysA = [];
        const keysB = [];
        a.forEach(function(valueA, keyA) {
            keysA.push(keyA);
        });
        b.forEach(function(valueB, keyB) {
            keysB.push(keyB);
        });
        const mapKeys = [keysA, keysB];
        const cmpKeys = [keysB, keysA];
        for (let i = 0; result && i < mapKeys.length; i++) {
            const mapIter = mapKeys[i];
            const cmpIter = cmpKeys[i];

            for (let j = 0; result && j < mapIter.length; j++) {
                const mapKey = mapIter[j];
                const cmpKey = cmpIter[j];
                const mapValueA = a.get(mapKey);
                let mapValueB;
                if (
                DeepTypeUtils.isAsymmetricEqualityTester_(mapKey) ||
                (DeepTypeUtils.isAsymmetricEqualityTester_(cmpKey) &&
                eq(mapKey, cmpKey))
                ) {
                    mapValueB = b.get(cmpKey);
                } else {
                    mapValueB = b.get(mapKey);
                }
                result = eq(mapValueA, mapValueB, aStack, bStack);
            }
        }
        if (!result) {
            return false;
        }
    } else if(DeepTypeUtils.isSet(a) && DeepTypeUtils.isSet(b)) {
        if (a.size != b.size) {
            return false;
        }
        const valuesA = [];
        a.forEach(function(valueA) {
            valuesA.push(valueA);
        });
        const valuesB = [];
        b.forEach(function(valueB) {
            valuesB.push(valueB);
        });
        const setPairs = [[valuesA, valuesB], [valuesB, valuesA]];
       const stackPairs = [[aStack, bStack], [bStack, aStack]];
        for (let i = 0; result && i < setPairs.length; i++) {
            const baseValues = setPairs[i][0];
            const otherValues = setPairs[i][1];
            const baseStack = stackPairs[i][0];
            const otherStack = stackPairs[i][1];
            for (const baseValue of baseValues) {
                let found = false;
                for (let j = 0; !found && j < otherValues.length; j++) {
                    const otherValue = otherValues[j];
                    const prevStackSize = baseStack.length;
                    found = eq(baseValue, otherValue, baseStack, otherStack);
                    if (!found && prevStackSize !== baseStack.length) {
                        baseStack.splice(prevStackSize);
                        otherStack.splice(prevStackSize);
                    }
                }
                result = result && found;
            }
        }
        if (!result) {
            return false;
        }
    } else {
        const aCtor = a.constructor,
            bCtor = b.constructor;
        if (
        aCtor !== bCtor &&
        DeepTypeUtils.isFunction_(aCtor) &&
        DeepTypeUtils.isFunction_(bCtor) &&
        a instanceof aCtor &&
        b instanceof bCtor &&
        !(aCtor instanceof aCtor && bCtor instanceof bCtor)
        ) {
            return false;
        }
    }

    const aKeys = DeepTypeUtils.keys(a, aClassName == '[object Array]');
    size = aKeys.length;

    if (DeepTypeUtils.keys(b, bClassName == '[object Array]').length !== size) {
        return false;
    }

    for (const key of aKeys) {
        console.log('key is:' + key);
        if(!DeepTypeUtils.has(b, key)) {
            result = false;
            continue;
        }
        if (!eq(a[key], b[key], aStack, bStack)) {
            result = false;
        }
    }
    if (!result) {
        return false;
    }
    aStack.pop();
    bStack.pop();
    return result;
}

function asymmetricMatch_(a, b) {
    const asymmetricA = DeepTypeUtils.isAsymmetricEqualityTester_(a);
    const asymmetricB = DeepTypeUtils.isAsymmetricEqualityTester_(b);

    if (asymmetricA === asymmetricB) {
        return undefined;
    }

}

/**
 * 获取对象的自有属性
 *
 * @param obj 对象
 * @param isArray 是否是一个数组
 */
function keys(obj, isArray) {
    const keys = [];

}

export default assertDeepEquals;
