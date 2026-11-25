#include <gtest/gtest.h>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#ifndef ESCX_SOURCE_DIR
#error ESCX_SOURCE_DIR must identify the ESCXML source tree
#endif
namespace {
std::vector<std::string> Split(const std::string& line)
{
    std::vector<std::string> fields; std::string field; std::istringstream input(line);
    while (std::getline(input, field, ',')) fields.push_back(field);
    return fields;
}
}
TEST(W3CIRPManifest, HasValidSchemaUniqueAssertionsAndExplicitDisposition)
{
    const std::string path = std::string(ESCX_SOURCE_DIR) + "/conformance/W3C_IRP_MANIFEST.csv";
    std::ifstream input(path.c_str()); ASSERT_TRUE(input.is_open()) << path;
    std::string line; ASSERT_TRUE(static_cast<bool>(std::getline(input, line)));
    EXPECT_EQ("assertion_id,test_id,spec_section,requirement,conformance,test_type,responsibility,disposition,evidence,next_stage", line);
    std::set<std::string> ids; std::size_t rows = 0U; std::size_t gaps = 0U;
    while (std::getline(input, line)) {
        if (line.empty()) continue;
        const std::vector<std::string> f = Split(line);
        ASSERT_EQ(10U, f.size()) << line;
        EXPECT_FALSE(f[0].empty()); EXPECT_FALSE(f[1].empty()); EXPECT_FALSE(f[2].empty());
        EXPECT_TRUE(ids.insert(f[0]).second) << "duplicate assertion " << f[0];
        EXPECT_TRUE(f[4] == "mandatory" || f[4] == "optional" || f[4] == "other");
        EXPECT_TRUE(f[5] == "automatic" || f[5] == "manual");
        EXPECT_TRUE(f[7] == "MAPPED" || f[7] == "PASS" || f[7] == "GAP" || f[7] == "PROFILE" || f[7] == "MANUAL");
        if (f[7] == "GAP") { ++gaps; EXPECT_NE("none", f[9]); }
        if (f[7] == "PASS") EXPECT_NE("none", f[8]);
        ++rows;
    }
    EXPECT_GE(rows, 40U);
    EXPECT_GE(gaps, 1U);
}
