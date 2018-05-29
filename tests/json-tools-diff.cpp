/*
 * Copyright © 2018 ÿystein Myrmo
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission. The copyright holders make no representations
 * about the suitability of this software for any purpose. It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include "json_tools_diff.h"
#include "assert.h"
#include <memory>

const char basicBaseJson[] = R"json(
{
    "name": "json",
    "id": 123,
    "value": 3.141592,
    "enabled": true,
    "meta": null
}
)json";

const char basicDiffJsonEqual[] = R"json(
{
    "enabled": true,
    "name": "json",
    "value": 3.1415921,
    "meta": null,
    "id": 123
}
)json";

const char basicDiffJsonDifferent[] = R"json(
{
    "enabled": false,
    "name": "xml",
    "value": 2.71828,
    "meta": "This is so meta",
    "id": 321
}
)json";

static int check_basic_object_diff()
{
    std::string baseJson(basicBaseJson);
    std::string diffJsonEqual(basicDiffJsonEqual);
    std::string diffJsonDifferent(basicDiffJsonDifferent);

    JT::DiffContext diffContext(baseJson);
    assert(diffContext.error == JT::DiffError::NoError);

    size_t diffPos = diffContext.diff(diffJsonEqual);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const& noDiff = diffContext.diffs[diffPos];
    assert(noDiff.diff_count == 0);

    diffPos = diffContext.diff(diffJsonDifferent);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const &allDiff = diffContext.diffs[diffPos];
    assert(allDiff.diff_count == 5);
    assert(allDiff.diffs.size() == 7);
    assert(allDiff.diffs[0] == JT::DiffType::NoDiff);
    assert(allDiff.diffs[1] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[2] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[3] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[4] == JT::DiffType::TypeDiff);
    assert(allDiff.diffs[5] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[6] == JT::DiffType::NoDiff);

    return 0;
}

const char emptyString[] = R"json()json";
const char emptyObjectJson[] = R"json({})json";
const char emptyArrayJson[] = R"json([])json";

static int check_empty_items()
{
    std::string empty(emptyString);
    std::string emptyObject(emptyObjectJson);
    std::string emptyArray(emptyArrayJson);
    size_t diffPos = -1;

// -----

    JT::DiffContext emptyDiffContext(empty);
    assert(emptyDiffContext.error == JT::DiffError::EmptyString);

    diffPos = emptyDiffContext.diff(empty);
    assert(diffPos == -1);
    assert(emptyDiffContext.error == JT::DiffError::EmptyString);

    diffPos = emptyDiffContext.diff(emptyObject);
    assert(diffPos == 0);
    assert(emptyDiffContext.error == JT::DiffError::NoError);

    diffPos = emptyDiffContext.diff(emptyArray);
    assert(diffPos == 1);
    assert(emptyDiffContext.error == JT::DiffError::NoError);

// -----

    JT::DiffContext emptyObjectDiffContext(emptyObject);
    assert(emptyObjectDiffContext.error == JT::DiffError::NoError);

    diffPos = emptyObjectDiffContext.diff(empty);
    assert(diffPos == -1);
    assert(emptyObjectDiffContext.error == JT::DiffError::EmptyString);

    diffPos = emptyObjectDiffContext.diff(emptyObject);
    assert(diffPos == 0);
    assert(emptyObjectDiffContext.error == JT::DiffError::NoError);
    assert(emptyObjectDiffContext.diffs[diffPos].diffs[0] == JT::DiffType::NoDiff);
    assert(emptyObjectDiffContext.diffs[diffPos].diffs[1] == JT::DiffType::NoDiff);

    diffPos = emptyObjectDiffContext.diff(emptyArray);
    assert(diffPos == 1);
    assert(emptyObjectDiffContext.error == JT::DiffError::NoError);
    assert(emptyObjectDiffContext.diffs[diffPos].diffs[0] == JT::DiffType::RootItemDiff);
    assert(emptyObjectDiffContext.diffs[diffPos].diffs[1] == JT::DiffType::RootItemDiff);

// -----

    JT::DiffContext emptyArrayDiffContext(emptyArray);
    assert(emptyArrayDiffContext.error == JT::DiffError::NoError);

    diffPos = emptyArrayDiffContext.diff(empty);
    assert(diffPos == -1);
    assert(emptyArrayDiffContext.error == JT::DiffError::EmptyString);

    diffPos = emptyArrayDiffContext.diff(emptyObject);
    assert(diffPos == 0);
    assert(emptyArrayDiffContext.error == JT::DiffError::NoError);
    assert(emptyArrayDiffContext.diffs[diffPos].diffs[0] == JT::DiffType::RootItemDiff);
    assert(emptyArrayDiffContext.diffs[diffPos].diffs[1] == JT::DiffType::RootItemDiff);

    diffPos = emptyArrayDiffContext.diff(emptyArray);
    assert(diffPos == 1);
    assert(emptyArrayDiffContext.error == JT::DiffError::NoError);
    assert(emptyArrayDiffContext.diffs[diffPos].diffs[0] == JT::DiffType::NoDiff);
    assert(emptyArrayDiffContext.diffs[diffPos].diffs[1] == JT::DiffType::NoDiff);

// -----

    return 0;
}

const char basicDiffOptionsJson[] = R"json(
{
    "name": "json",
    "id": 123,
    "value": 3.141592,
    "enabled": true,
    "meta": null,
    "number1": 1.000000001,
    "number2": 2.000000020,
    "number3": 3.000000300,
    "number4": 4.000004000,
    "number5": 5.000050000,
    "number6": 6.000600000,
    "number7": 7.007000000,
    "number8": 8.080000000,
    "number9": 9.900000000,
    "number10": 10.00000000
}
)json";

const char basicDiffOptionsJsonIdentical[] = R"json(
{
    "name": "json",
    "id": 123,
    "value": 3.141592,
    "enabled": true,
    "meta": null,
    "number1": 1.000000001,
    "number2": 2.000000020,
    "number3": 3.000000300,
    "number4": 4.000004000,
    "number5": 5.000050000,
    "number6": 6.000600000,
    "number7": 7.007000000,
    "number8": 8.080000000,
    "number9": 9.900000000,
    "number10": 10.00000000
}
)json";

const char basicDiffOptionsJsonAlmostEqual[] = R"json(
{
    "name": "json",
    "id": 123,
    "value": 3.141592,
    "enabled": true,
    "meta": null,
    "number1": 1.0000000011,
    "number2": 2.0000000220,
    "number3": 3.0000003300,
    "number4": 4.0000044000,
    "number5": 5.0000550000,
    "number6": 6.0006600000,
    "number7": 7.0077000000,
    "number8": 8.0880000000,
    "number9": 9.9900000000,
    "number10": 11.00000000
}
)json";

static int check_diff_options()
{
    std::string jsonBase(basicDiffOptionsJson);
    std::string jsonIdentical(basicDiffOptionsJsonIdentical);
    std::string jsonAlmostEqual(basicDiffOptionsJsonAlmostEqual);

    JT::DiffOptions options = { JT::DiffFlags::None, 0 };

    JT::DiffContext diffContext(jsonBase, options);
    assert(diffContext.error == JT::DiffError::NoError);
    size_t diffPos;

    // JT::DiffFlags::None
    diffPos = diffContext.diff(jsonIdentical);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const &noDiffNoFuzzy = diffContext.diffs[diffPos];
    assert(noDiffNoFuzzy.error == JT::DiffError::NoError);
    assert(noDiffNoFuzzy.diff_count == 0);

    // JT::DiffFlags::FuzzyFloatComparison / 0.0
    diffContext.options = { JT::DiffFlags::FuzzyFloatComparison, 0.0 };
    diffPos = diffContext.diff(jsonIdentical);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const &noDiffFuzzy = diffContext.diffs[diffPos];
    assert(noDiffFuzzy.error == JT::DiffError::NoError);
    assert(noDiffFuzzy.diff_count == 0);

    // JT::DiffFlags::FuzzyFloatComparison / 1e-9
    diffContext.options = { JT::DiffFlags::FuzzyFloatComparison, 1e-9 };
    diffPos = diffContext.diff(jsonAlmostEqual);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const &diff_9 = diffContext.diffs[diffPos];
    assert(diff_9.error == JT::DiffError::NoError);
    assert(diff_9.diff_count == 9);

    // JT::DiffFlags::FuzzyFloatComparison / 1e-8
    diffContext.options = { JT::DiffFlags::FuzzyFloatComparison, 1e-8 };
    diffPos = diffContext.diff(jsonAlmostEqual);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const &diff_8 = diffContext.diffs[diffPos];
    assert(diff_8.error == JT::DiffError::NoError);
    assert(diff_8.diff_count == 8);

    // JT::DiffFlags::FuzzyFloatComparison / 1e-7
    diffContext.options = { JT::DiffFlags::FuzzyFloatComparison, 1e-7 };
    diffPos = diffContext.diff(jsonAlmostEqual);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const &diff_7 = diffContext.diffs[diffPos];
    assert(diff_7.error == JT::DiffError::NoError);
    assert(diff_7.diff_count == 7);

    // JT::DiffFlags::FuzzyFloatComparison / 1e-6
    diffContext.options = { JT::DiffFlags::FuzzyFloatComparison, 1e-6 };
    diffPos = diffContext.diff(jsonAlmostEqual);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const &diff_6 = diffContext.diffs[diffPos];
    assert(diff_6.error == JT::DiffError::NoError);
    assert(diff_6.diff_count == 6);

    // JT::DiffFlags::FuzzyFloatComparison / 1e-5
    diffContext.options = { JT::DiffFlags::FuzzyFloatComparison, 1e-5 };
    diffPos = diffContext.diff(jsonAlmostEqual);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const &diff_5 = diffContext.diffs[diffPos];
    assert(diff_5.error == JT::DiffError::NoError);
    assert(diff_5.diff_count == 5);

    // JT::DiffFlags::FuzzyFloatComparison / 1e-4
    diffContext.options = { JT::DiffFlags::FuzzyFloatComparison, 1e-4 };
    diffPos = diffContext.diff(jsonAlmostEqual);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const &diff_4 = diffContext.diffs[diffPos];
    assert(diff_4.error == JT::DiffError::NoError);
    assert(diff_4.diff_count == 4);

    // JT::DiffFlags::FuzzyFloatComparison / 1e-3
    diffContext.options = { JT::DiffFlags::FuzzyFloatComparison, 1e-3 };
    diffPos = diffContext.diff(jsonAlmostEqual);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const &diff_3 = diffContext.diffs[diffPos];
    assert(diff_3.error == JT::DiffError::NoError);
    assert(diff_3.diff_count == 3);

    // JT::DiffFlags::FuzzyFloatComparison / 1e-2
    diffContext.options = { JT::DiffFlags::FuzzyFloatComparison, 1e-2 };
    diffPos = diffContext.diff(jsonAlmostEqual);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const &diff_2 = diffContext.diffs[diffPos];
    assert(diff_2.error == JT::DiffError::NoError);
    assert(diff_2.diff_count == 2);

    // JT::DiffFlags::FuzzyFloatComparison / 1e-1
    diffContext.options = { JT::DiffFlags::FuzzyFloatComparison, 1e-1 };
    diffPos = diffContext.diff(jsonAlmostEqual);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const &diff_1 = diffContext.diffs[diffPos];
    assert(diff_1.error == JT::DiffError::NoError);
    assert(diff_1.diff_count == 1);

    return 0;
}

const char basicBaseJsonWithSubObject[] = R"json(
{
    "name": "json",
    "id": 123,
    "value": 3.141592,
    "enabled": true,
    "meta": null,
    "sub_object": {
        "name": "json_sub",
        "id": 1234,
        "value": 1.570796,
        "enabled": false,
        "meta": null
    }
}
)json";

const char basicDiffJsonEqualWithSubObject[] = R"json(
{
    "enabled": true,
    "name": "json",
    "sub_object": {
        "meta": null,
        "value": 1.570796,
        "id": 1234,
        "enabled": false,
        "name": "json_sub"
    },
    "value": 3.1415921,
    "meta": null,
    "id": 123
}
)json";

const char basicDiffJsonDifferentWithSubObject[] = R"json(
{
    "enabled": false,
    "name": "xml",
    "sub_object": {
        "meta": 067,
        "value": 1.35914,
        "id": 1234,
        "enabled": true,
        "name": "xml_sub"
    },
    "value": 2.71828,
    "meta": "This is so meta",
    "id": 321
}
)json";

static int check_nested_objects_diff()
{
    std::string baseJson(basicBaseJsonWithSubObject);
    std::string diffJsonEqual(basicDiffJsonEqualWithSubObject);
    std::string diffJsonDifferent(basicDiffJsonDifferentWithSubObject);

    JT::DiffContext diffContext(baseJson);
    assert(diffContext.error == JT::DiffError::NoError);

    size_t diffPos = diffContext.diff(diffJsonEqual);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const& noDiff = diffContext.diffs[diffPos];
    assert(noDiff.diff_count == 0);

    diffPos = diffContext.diff(diffJsonDifferent);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const &allDiff = diffContext.diffs[diffPos];
    assert(allDiff.diff_count == 9);
    assert(allDiff.diffs.size() == 14);
    assert(allDiff.diffs[0] == JT::DiffType::NoDiff);
    assert(allDiff.diffs[1] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[2] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[3] == JT::DiffType::NoDiff);
    assert(allDiff.diffs[4] == JT::DiffType::TypeDiff);
    assert(allDiff.diffs[5] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[6] == JT::DiffType::NoDiff);
    assert(allDiff.diffs[7] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[8] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[9] == JT::DiffType::NoDiff);
    assert(allDiff.diffs[10] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[11] == JT::DiffType::TypeDiff);
    assert(allDiff.diffs[12] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[13] == JT::DiffType::NoDiff);

    diffContext.changeBase(diffPos);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const &allDiffNewBase = diffContext.diffs[diffPos];
    assert(allDiffNewBase.diff_count == 9);
    assert(allDiff.diffs[0] == JT::DiffType::NoDiff);
    assert(allDiff.diffs[1] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[2] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[3] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[4] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[5] == JT::DiffType::TypeDiff);
    assert(allDiff.diffs[6] == JT::DiffType::NoDiff);
    assert(allDiff.diffs[7] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[8] == JT::DiffType::NoDiff);
    assert(allDiff.diffs[9] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[10] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[11] == JT::DiffType::TypeDiff);
    assert(allDiff.diffs[12] == JT::DiffType::NoDiff);
    assert(allDiff.diffs[13] == JT::DiffType::NoDiff);

    return 0;
}

const char basicArrayJson[] = R"json(
[
    {
        "name": "item0",
        "id": 0,
        "value": 1.0,
        "enabled": true,
        "meta": null
    },
    {
        "name": "item1",
        "id": 1,
        "value": 2.0,
        "enabled": false,
        "meta": "I am meta"
    },
    {
        "name": "item2",
        "id": 2,
        "value": 4.2e1,
        "enabled": true,
        "meta": 42
    }
]
)json";

const char basicArrayJsonEqual[] = R"json(
[
    {
        "meta": null,
        "value": 1.0000,
        "id": 0,
        "name": "item0",
        "enabled": true
    },
    {
        "value": 2.0,
        "id": 1,
        "meta": "I am meta",
        "name": "item1",
        "enabled": false
    },
    {
        "name": "item2",
        "id": 2,
        "value": 4.2e1,
        "enabled": true,
        "meta": 42
    }
]
)json";

const char basicArrayJsonDifferent[] = R"json(
[
    {
        "value": 1e1,
        "meta": "meta10",
        "name": "item10",
        "enabled": false,
        "id": 10
    },
    {
        "meta": "I am more meta",
        "value": 20.0,
        "name": "item2",
        "id": 20,
        "enabled": false
    },
    {
        "name": "item20",
        "id": 24,
        "value": 2.4e1,
        "enabled": true,
        "meta": 24
    }
]
)json";

int check_basic_array_diff_ordered()
{
    std::string baseJson(basicArrayJson);
    std::string diffJsonEqual(basicArrayJsonEqual);
    std::string diffJsonDifferent(basicArrayJsonDifferent);

    JT::DiffContext diffContext(baseJson);
    assert(diffContext.error == JT::DiffError::NoError);

    size_t diffPos = diffContext.diff(diffJsonEqual);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const& noDiff = diffContext.diffs[diffPos];
    assert(noDiff.diff_count == 0);

    diffPos = diffContext.diff(diffJsonDifferent);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const &allDiff = diffContext.diffs[diffPos];
    assert(allDiff.diff_count == 13);
    assert(allDiff.size() == 23);
    assert(allDiff.diffs[0] == JT::DiffType::NoDiff);
    assert(allDiff.diffs[1] == JT::DiffType::NoDiff);
    assert(allDiff.diffs[2] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[3] == JT::DiffType::TypeDiff);
    assert(allDiff.diffs[4] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[5] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[6] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[7] == JT::DiffType::NoDiff);
    assert(allDiff.diffs[8] == JT::DiffType::NoDiff);
    assert(allDiff.diffs[9] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[10] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[11] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[12] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[13] == JT::DiffType::NoDiff);
    assert(allDiff.diffs[14] == JT::DiffType::NoDiff);
    assert(allDiff.diffs[15] == JT::DiffType::NoDiff);
    assert(allDiff.diffs[16] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[17] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[18] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[19] == JT::DiffType::NoDiff);
    assert(allDiff.diffs[20] == JT::DiffType::ValueDiff);
    assert(allDiff.diffs[21] == JT::DiffType::NoDiff);
    assert(allDiff.diffs[22] == JT::DiffType::NoDiff);

    return 0;
}

const char largeObjectWithAllDataTypes[] = R"json(
{
    "member1": 0,
    "member2": 2147483647,
    "member3": -2147483648,
    "member4": 0e10,
    "member5": 2e5,
    "member6": 2E5,
    "member7": 2e+5,
    "member8": 2E+5,
    "member9": 2e-5,
    "member10": 2E-5,
    "member11": -3e4,
    "member12": -3E4,
    "member13": -3e+4,
    "member14": -3E+4,
    "member15": -3e-4,
    "member16": -3E-4,
    "member17": 0.0,
    "member18": 123456.789,
    "member19": -123456.789,
    "member20": 2.345e5,
    "member21": 2.345E5,
    "member22": -3.456e5,
    "member23": -3.456E5,
    "member24": "iamastring",
    "member25": "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ \" \\\/\b\f\n\r\t!@#$%^&*()_+-=[]{};:',.<>Ê¯Â∆ÿ≈‡·‚‰ËÈÍÎÚÛÙÙ",
    "member26": "",
    "member27": {},
    "member28": [],
    "member29": [[],{},[],{}],
    "member30": [[[[[[[[[[null,true,false,1,2,3,"a","b","c"]]]]]]]]]],
    "member31": null,
    "member32": true,
    "member33": false
}
)json";

const char largeObjectWithAllDataTypesEqual[] = R"json(
{
    "member1": 0,
    "member2": 2147483647,
    "member3": -2147483648,
    "member4": 0e10,
    "member5": 2e5,
    "member6": 2E5,
    "member7": 2e+5,
    "member8": 2E+5,
    "member9": 2e-5,
    "member10": 2E-5,
    "member11": -3e4,
    "member12": -3E4,
    "member13": -3e+4,
    "member14": -3E+4,
    "member15": -3e-4,
    "member16": -3E-4,
    "member17": 0.0,
    "member18": 123456.789,
    "member19": -123456.789,
    "member20": 2.345e5,
    "member21": 2.345E5,
    "member22": -3.456e5,
    "member23": -3.456E5,
    "member24": "iamastring",
    "member25": "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ \" \\\/\b\f\n\r\t!@#$%^&*()_+-=[]{};:',.<>Ê¯Â∆ÿ≈‡·‚‰ËÈÍÎÚÛÙÙ",
    "member26": "",
    "member27": {},
    "member28": [],
    "member29": [[],{},[],{}],
    "member30": [[[[[[[[[[null,true,false,1,2,3,"a","b","c"]]]]]]]]]],
    "member31": null,
    "member32": true,
    "member33": false
}
)json";

const char largeObjectWithAllDataTypesDifferent[] = R"json(
{
    "member1": 0,
    "member2": 2147483647,
    "member3": -2147483648,
    "member4": 0e10,
    "member5": 0.2e6,
    "member6": 0.2E6,
    "member7": 2e+5,
    "member8": 2E+5,
    "member9": 2e-5,
    "member10": 2E-5,
    "member11": -0.03e6,
    "member12": -0.003E7,
    "member13": -3e+4,
    "member14": -3E+4,
    "member15": -3e-4,
    "member16": -3E-4,
    "member17": 0.0,
    "member18": 123456.789,
    "member19": -123456.789,
    "member20": 2.345e5,
    "member21": 2.345E5,
    "member22": -3.456e5,
    "member23": -3.456E5,
    "member24": "iamastring",
    "member25": "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ \" \\\/\b\f\n\r\t!@#$%^&*()_+-=[]{};:',.<>Ê¯Â∆ÿ≈‡·‚‰ËÈÍÎÚÛÙÙ",
    "member26": "",
    "member27": {},
    "member28": [],
    "member29": [[],{},[],{}],
    "member30": [[[[[[[[[[true,false,1,2,3,"a","b","c",1,"hey"]]]]]]]]]],
    "member31": null,
    "member32": true,
    "member33": false
}
)json";

int check_large_object_with_all_data_types()
{
    std::string baseJson(largeObjectWithAllDataTypes);
    std::string diffJsonEqual(largeObjectWithAllDataTypesEqual);
    std::string diffJsonDifferent(largeObjectWithAllDataTypesDifferent);

    JT::DiffContext diffContext(baseJson);
    assert(diffContext.error == JT::DiffError::NoError);

    size_t diffPos = diffContext.diff(diffJsonEqual);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const& noDiff = diffContext.diffs[diffPos];
    assert(noDiff.diff_count == 0);

    diffPos = diffContext.diff(diffJsonDifferent);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const &diff = diffContext.diffs[diffPos];
    assert(diff.diff_count == 10);
    assert(diff.size() == 75);
    assert(diff.diffs[51] == JT::DiffType::TypeDiff);
    assert(diff.diffs[52] == JT::DiffType::ValueDiff);
    assert(diff.diffs[53] == JT::DiffType::TypeDiff);
    assert(diff.diffs[54] == JT::DiffType::ValueDiff);
    assert(diff.diffs[55] == JT::DiffType::ValueDiff);
    assert(diff.diffs[56] == JT::DiffType::TypeDiff);
    assert(diff.diffs[57] == JT::DiffType::ValueDiff);
    assert(diff.diffs[58] == JT::DiffType::ValueDiff);
    assert(diff.diffs[59] == JT::DiffType::TypeDiff);
    assert(diff.diffs[60] == JT::DiffType::NewArrayItem);

    diffContext.changeBase(diffPos);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const &diffNewBase = diffContext.diffs[diffPos];
    assert(diffNewBase.diff_count == 10);
    assert(diffNewBase.size() == 74);
    assert(diffNewBase.diffs[50] == JT::DiffType::MissingArrayItems);
    assert(diffNewBase.diffs[51] == JT::DiffType::TypeDiff);
    assert(diffNewBase.diffs[52] == JT::DiffType::ValueDiff);
    assert(diffNewBase.diffs[53] == JT::DiffType::TypeDiff);
    assert(diffNewBase.diffs[54] == JT::DiffType::ValueDiff);
    assert(diffNewBase.diffs[55] == JT::DiffType::ValueDiff);
    assert(diffNewBase.diffs[56] == JT::DiffType::TypeDiff);
    assert(diffNewBase.diffs[57] == JT::DiffType::ValueDiff);
    assert(diffNewBase.diffs[58] == JT::DiffType::ValueDiff);
    assert(diffNewBase.diffs[59] == JT::DiffType::TypeDiff);

    return 0;
}

const char jsonWithSubObjects[] = R"json(
{
    "member1": 1,
    "member2": 2,
    "member3": 3,
    "sub_object": {
        "submember1": "11",
        "submember2": "22",
        "subsub_object": {
            "subsubmember1": 1.1e1,
            "subsubmember2": 2.2e1,
            "subsubsub_array": [
                true,
                false,
                "A",
                "B",
                "C",
            ],
            "subsubsub_object": {
                "id": 333,
                "name": "Sub SubSub",
                "info": null,
                "array_with_missing_members": [
                    4,
                    5,
                    6,
                    7,
                    {
                        "i": "am",
                        "a": "missing_object"
                    }
                ]
            }
        },
        "subsub_array": [
            "string",
            123,
            true,
            false,
            null,
            {
                "subsub_array_object": {
                    "item1": 1,
                    "itemA": "A",
                    "itemNull": null
                }
            }
        ]
    }
}
)json";

const char jsonWithSubObjects_MissingAndNewMembers[] = R"json(
{
    "member4": 4,
    "memberG": "G",
    "member1": 1,
    "sub_object": {
        "submember2": "22",
        "submember3": "33",
        "another_member": null,
        "subsub_array": [
            123,
            "string",
            true,
            false,
            null,
            {
                "subsub_array_object": {
                    "itemA": "A",
                    "new_item": "new",
                    "itemNull": null,
                    "another_new_item": 1.23456e7
                }
            }
        ],
        "subsub_object": {
            "subsubsub_array": [
                true,
                false,
                "B",
                "A",
                "C",
                "new_array_item",
                555
            ],
            "subsubmember1": 1.1e1,
            "subsubmember2": 2.2e1,
            "subsubsub_object": {
                "id": 333,
                "name": "New name",
                "info": {
                    "mem1": 1,
                    "mem2": 2,
                    "mem3": 3
                },
                "array_with_missing_members": [
                    4,
                    5,
                    6
                ]
            }
        }
    }
}
)json";

int check_new_and_missing_members()
{
    std::string baseJson(jsonWithSubObjects);
    std::string diffJsonWithMissingAndNewMembers(jsonWithSubObjects_MissingAndNewMembers);

    JT::DiffContext diffContext(baseJson);
    assert(diffContext.error == JT::DiffError::NoError);

    size_t diffPos = diffContext.diff(diffJsonWithMissingAndNewMembers);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const& diff = diffContext.diffs[diffPos];
    assert(diff.diff_count == 27);

    // Assert missing members.
    assert(diff.missingMembers.size() == 3);
    JT::Token token;

    const JT::Token &missingMembers1 = diff.tokens.data[0];
    const std::vector<JT::Token>* missing1 = diff.getMissingMembers(missingMembers1);
    assert(missing1->size() == 2);
    token = (*missing1)[0];
    assert(strncmp(token.name.data, "member2", token.name.size) == 0);
    assert(token.value_type == JT::Type::Number);
    token = (*missing1)[1];
    assert(strncmp(token.name.data, "member3", token.name.size) == 0);
    assert(token.value_type == JT::Type::Number);

    const JT::Token &missingMembers2 = diff.tokens.data[4];
    const std::vector<JT::Token>* missing2 = diff.getMissingMembers(missingMembers2);
    assert(missing2->size() == 1);
    token = (*missing2)[0];
    assert(strncmp(token.name.data, "submember1", token.name.size) == 0);
    assert(token.value_type == JT::Type::String);

    const JT::Token &missingMembers3 = diff.tokens.data[15];
    const std::vector<JT::Token>* missing3 = diff.getMissingMembers(missingMembers3);
    assert(missing3->size() == 1);
    token = (*missing3)[0];
    assert(strncmp(token.name.data, "item1", token.name.size) == 0);
    assert(token.value_type == JT::Type::Number);

    const JT::Token &tokenWithoutMissingMembers = diff.tokens.data[1];
    const std::vector<JT::Token>* missingNone = diff.getMissingMembers(tokenWithoutMissingMembers);
    assert(missingNone == nullptr);

    // Assert missing array items.
    assert(diff.missingArrayItems.size() == 1);

    const JT::Token &missingArrayItemsToken = diff.tokens.data[43];
    const std::vector<JT::Token>* missingArrayItems = diff.getMissingArrayItems(missingArrayItemsToken);
    assert(missingArrayItems->size() == 5);
    token = (*missingArrayItems)[0];
    assert(token.value_type == JT::Type::Number);
    assert(strncmp(token.value.data, "7", token.value.size) == 0);
    token = (*missingArrayItems)[1];
    assert(token.value_type == JT::Type::ObjectStart);
    token = (*missingArrayItems)[2];
    assert(token.value_type == JT::Type::String);
    assert(strncmp(token.name.data, "i", token.name.size) == 0);
    assert(strncmp(token.value.data, "am", token.value.size) == 0);
    token = (*missingArrayItems)[3];
    assert(token.value_type == JT::Type::String);
    assert(strncmp(token.name.data, "a", token.name.size) == 0);
    assert(strncmp(token.value.data, "missing_object", token.value.size) == 0);
    token = (*missingArrayItems)[4];
    assert(token.value_type == JT::Type::ObjectEnd);

    return 0;
}

const char jsonMissingMembersAndArrayItemsBase[] = R"json(
{
    "object1": {
        "array11": [
            {
                "array111": [
                    1,
                    2,
                    3,
                    {
                        "key": "val"
                    }
                ],
                "object111": {
                    "miss111": {
                        "a": "b",
                        "c": 3,
                        "d": false,
                        "e": null,
                        "f": 3.141592
                    }
                }
            },
            1,
            2,
            "3"
         ],
        "object11": {
            "array112": [
                1,
                2,
                3
            ]
        }
    },
    "array1": [
        "a",
        {
            "object12": {
                "key1": "val1",
                "key2": {
                    "mem1": 1,
                    "mem2": false,
                    "array121": [
                        1,
                        2,
                        3
                    ]
                }
            }
        },
        "b",
        [
            {
                "anon_obj_key1": 123,
                "anon_obj_key2": [
                    "a",
                    "b",
                    "c",
                    "d"
                ]
            }
        ]
    ]
}
)json";

const char jsonMissingMembersAndArrayItemsDiff[] = R"json(
{
    "object1": {
        "array11": [
            {
                "array111": [
                    1,
                    3
                ],
                "object111": {
                    "miss111_changed": {
                        "a": "b",
                        "e": null
                    }
                }
            },
            1
         ],
        "object11": {
            "array112": [
                1,
                2,
                3,
                {
                    "this_is_a_new": "object",
                    "new_array": [
                        "new",
                        "array"
                    ]
                },
                null
            ]
        }
    },
    "array1": [
        "a",
        {
            "object12": {
                "key1": "val1",
                "key2": {
                    "array_new": [
                        0.1,
                        0.2,
                        0.3
                    ],
                    "array121": [
                        1,
                        2,
                        3
                    ]
                }
            }
        },
        "b",
        [
            {
                "anon_obj_key1": 123,
                "anon_obj_key2": [
                    "d"
                ]
            },
            "new",
            456,
            false
        ]
    ]
}
)json";

int check_new_and_missing_members_nested()
{
    std::string baseJson(jsonMissingMembersAndArrayItemsBase);
    std::string diffJson(jsonMissingMembersAndArrayItemsDiff);

    JT::DiffContext diffContext(baseJson);
    assert(diffContext.error == JT::DiffError::NoError);

    size_t diffPos = diffContext.diff(diffJson);
    assert(diffContext.error == JT::DiffError::NoError);
    auto const& diff = diffContext.diffs[diffPos];
    assert(diff.diff_count == 40);

    // Assert missing members.
    assert(diff.missingMembers.size() == 2);
    JT::Token token;

    const JT::Token& missingMembers1 = diff.tokens.data[8];
    const std::vector<JT::Token>* members1 = diff.getMissingMembers(missingMembers1);
    assert(members1->size() == 7);
    token = (*members1)[0];
    assert(token.value_type == JT::Type::ObjectStart);
    assert(strncmp(token.name.data, "miss111", token.name.size) == 0);
    token = (*members1)[1];
    assert(token.value_type == JT::Type::String);
    assert(strncmp(token.name.data, "a", token.name.size) == 0);
    assert(strncmp(token.value.data, "b", token.value.size) == 0);
    token = (*members1)[2];
    assert(token.value_type == JT::Type::Number);
    assert(strncmp(token.name.data, "c", token.name.size) == 0);
    assert(strncmp(token.value.data, "3", token.value.size) == 0);
    token = (*members1)[3];
    assert(token.value_type == JT::Type::Bool);
    assert(strncmp(token.name.data, "d", token.name.size) == 0);
    assert(strncmp(token.value.data, "false", token.value.size) == 0);
    token = (*members1)[4];
    assert(token.value_type == JT::Type::Null);
    assert(strncmp(token.name.data, "e", token.name.size) == 0);
    assert(strncmp(token.value.data, "null", token.value.size) == 0);
    token = (*members1)[5];
    assert(token.value_type == JT::Type::Number);
    assert(strncmp(token.name.data, "f", token.name.size) == 0);
    // Note actual float not compared.
    token = (*members1)[6];
    assert(token.value_type == JT::Type::ObjectEnd);

    const JT::Token& missingMembers2 = diff.tokens.data[38];
    const std::vector<JT::Token>* members2 = diff.getMissingMembers(missingMembers2);
    assert(members2->size() == 2);
    token = (*members2)[0];
    assert(token.value_type == JT::Type::Number);
    assert(strncmp(token.name.data, "mem1", token.name.size) == 0);
    assert(strncmp(token.value.data, "1", token.value.size) == 0);
    token = (*members2)[1];
    assert(token.value_type == JT::Type::Bool);
    assert(strncmp(token.name.data, "mem2", token.name.size) == 0);
    assert(strncmp(token.value.data, "false", token.value.size) == 0);

    // Assert array items.
    assert(diff.missingArrayItems.size() == 3);

    const JT::Token& missingArrayItems1 = diff.tokens.data[2];
    const std::vector<JT::Token>* missing1 = diff.getMissingArrayItems(missingArrayItems1);
    assert(missing1->size() == 2);
    token = (*missing1)[0];
    assert(token.value_type == JT::Type::Number);
    assert(strncmp(token.value.data, "2", token.value.size) == 0);
    token = (*missing1)[1];
    assert(token.value_type == JT::Type::String);
    assert(strncmp(token.value.data, "3", token.name.size) == 0);

    const JT::Token& missingArrayItems2 = diff.tokens.data[4];
    const std::vector<JT::Token>* missing2 = diff.getMissingArrayItems(missingArrayItems2);
    assert(missing2->size() == 4);
    token = (*missing2)[0];
    assert(token.value_type == JT::Type::Number);
    assert(strncmp(token.value.data, "3", token.name.size) == 0);
    token = (*missing2)[1];
    assert(token.value_type == JT::Type::ObjectStart);
    token = (*missing2)[2];
    assert(token.value_type == JT::Type::String);
    assert(strncmp(token.name.data, "key", token.name.size) == 0);
    assert(strncmp(token.value.data, "val", token.value.size) == 0);
    token = (*missing2)[3];
    assert(token.value_type == JT::Type::ObjectEnd);

    const JT::Token& missingArrayItems3 = diff.tokens.data[56];
    const std::vector<JT::Token>* missing3 = diff.getMissingArrayItems(missingArrayItems3);
    assert(missing3->size() == 3);
    token = (*missing3)[0];
    assert(token.value_type == JT::Type::String);
    assert(strncmp(token.value.data, "b", token.value.size) == 0);
    token = (*missing3)[1];
    assert(token.value_type == JT::Type::String);
    assert(strncmp(token.value.data, "c", token.value.size) == 0);
    token = (*missing3)[2];
    assert(token.value_type == JT::Type::String);
    assert(strncmp(token.value.data, "d", token.value.size) == 0);

    return 0;
}

int main()
{
    check_basic_object_diff();
    check_empty_items();
    check_diff_options();
    check_nested_objects_diff();
    check_basic_array_diff_ordered();
    check_large_object_with_all_data_types();
    check_new_and_missing_members();
    check_new_and_missing_members_nested();

    return 0;
}
