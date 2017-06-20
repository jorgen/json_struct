#include <chrono>
#include <json_tools.h>
#include "assert.h"
#include "generated.json.h"

#include "include/rapidjson/document.h"
#include <json/json.h>
#include "include/sajson/sajson.h"

struct BenchmarkRun
{
    virtual ~BenchmarkRun() { }
    virtual std::string name() const = 0;
    virtual void run(const std::string &json) = 0;

    int max_size;
    std::string second_name;
};

void runBenchmark(const std::string &json, BenchmarkRun &benchmark)
{
    auto start= std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000; i++)
        benchmark.run(json);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = end - start;
    fprintf(stderr, "Execution of %s: size is %d and it took %f. Name is %s\n", benchmark.name().c_str(), benchmark.max_size, ms.count(), benchmark.second_name.c_str());
}

struct JsonTokenizerRun : public BenchmarkRun
{
    std::string name() const override { return "JsonTools tokenizer"; }
    void run(const std::string &json) override
    {
        JT::Tokenizer tokenizer;
        tokenizer.addData(json.c_str(), json.size());

        JT::Token token;
        JT::Error error = JT::Error::NoError;
        int array_size = 0;
        max_size = 0;
        do
        {
            error = tokenizer.nextToken(token);
            if (token.value_type == JT::Type::ArrayStart) {
                array_size++;
            }
            else if (token.value_type == JT::Type::ArrayEnd) {
                array_size--;
            } else if (token.value_type == JT::Type::ObjectStart && array_size == 1)
                max_size++;
        }
        while(array_size > 0 && error == JT::Error::NoError);


        if (error != JT::Error::NoError)
            fprintf(stderr, "Failed to parse document\n");
    }
};

struct JsonToolsStructRun : public BenchmarkRun
{
    std::string name() const override { return "JsonTools struct"; }
    void run(const std::string &json) override
    {
        JT::ParseContext context(json.c_str(), json.size());
        std::vector<JPerson> people;
        context.parseTo(people);
        max_size = people.size();
        second_name = people[1].name;
    }
};

struct RapidJsonRun : public BenchmarkRun
{
    std::string name() const override { return "RapidJson Dom"; }
    void run(const std::string &json) override
    {
        rapidjson::Document d;
        d.Parse(json.c_str());
        max_size = d.Size();
        second_name = d[1]["name"].GetString();
    }
};

struct OldJsonCpp : public BenchmarkRun
{
    std::string name() const override { return "Old JsonCPP"; }
    void run(const std::string &json) override
    {
        Json::Value v;
        Json::Reader().parse(json, v);
        max_size = v.size();
        second_name = v[1]["name"].asString();
    }
};

struct SaJsonRun : public BenchmarkRun
{
    std::string name() const override { return "SaJson"; }
    void run(const std::string &json) override
    {
        const sajson::document& document = sajson::parse(sajson::dynamic_allocation(), sajson::string(json.c_str(), json.size()));
        if (!document.is_valid()) {
            fprintf(stderr, "Failed to parse Sajson\n");
        }
        max_size = document.get_root().get_length();
        second_name = document.get_root().get_array_element(1).get_value_of_key(sajson::literal("name")).as_string();
    }
};

int main()
{
    BenchmarkRun *run = new JsonTokenizerRun();
    runBenchmark(std::string(generatedJson), *run);
    delete run;
    //run = new JsonToolsStructRun();
    //runBenchmark(std::string(generatedJson), *run);
    //delete run;
    //run = new RapidJsonRun();
    //runBenchmark(std::string(generatedJson), *run);
    //delete run;
    //run = new OldJsonCpp();
    //runBenchmark(std::string(generatedJson), *run);
    //delete run;
    //run = new SaJsonRun();
    //runBenchmark(std::string(generatedJson), *run);
    //delete run;



    return 0;
}
