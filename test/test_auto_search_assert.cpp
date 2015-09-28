#include <scope/test.h>

#include "stest.h"

SCOPE_FIXTURE_CTOR(autoPatternAssertTest0, STest, STest(R"(\A)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest1, STest, STest(R"(\Z)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest2, STest, STest(R"(\K)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest3, STest, STest(R"(\b)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest4, STest, STest(R"(\B)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest5, STest, STest(R"(^)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest6, STest, STest(R"($)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest7, STest, STest(R"((?<=a))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest8, STest, STest(R"((?<!a))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest9, STest, STest(R"((?=a))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest10, STest, STest(R"((?!a))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest11, STest, STest(R"((a))")) {
  const char text[] = R"(aaa
aaca

a
cac
acc


c
ccca)";
  fixture.search(text, text + 28, 0);
  SCOPE_ASSERT_EQUAL(10u, fixture.Hits.size());
  SCOPE_ASSERT_EQUAL(SearchHit(0, 1, 0), fixture.Hits[0]);
  SCOPE_ASSERT_EQUAL(SearchHit(1, 2, 0), fixture.Hits[1]);
  SCOPE_ASSERT_EQUAL(SearchHit(2, 3, 0), fixture.Hits[2]);
  SCOPE_ASSERT_EQUAL(SearchHit(4, 5, 0), fixture.Hits[3]);
  SCOPE_ASSERT_EQUAL(SearchHit(5, 6, 0), fixture.Hits[4]);
  SCOPE_ASSERT_EQUAL(SearchHit(7, 8, 0), fixture.Hits[5]);
  SCOPE_ASSERT_EQUAL(SearchHit(10, 11, 0), fixture.Hits[6]);
  SCOPE_ASSERT_EQUAL(SearchHit(13, 14, 0), fixture.Hits[7]);
  SCOPE_ASSERT_EQUAL(SearchHit(16, 17, 0), fixture.Hits[8]);
  SCOPE_ASSERT_EQUAL(SearchHit(27, 28, 0), fixture.Hits[9]);
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest12, STest, STest(R"((?<=\A))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest13, STest, STest(R"((?<!\A))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest14, STest, STest(R"((?=\A))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest15, STest, STest(R"((?!\A))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest16, STest, STest(R"((\A))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest17, STest, STest(R"((?<=\Z))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest18, STest, STest(R"((?<!\Z))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest19, STest, STest(R"((?=\Z))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest20, STest, STest(R"((?!\Z))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest21, STest, STest(R"((\Z))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest22, STest, STest(R"((?<=\K))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest23, STest, STest(R"((?<!\K))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest24, STest, STest(R"((?=\K))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest25, STest, STest(R"((?!\K))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest26, STest, STest(R"((\K))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest27, STest, STest(R"((?<=\b))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest28, STest, STest(R"((?<!\b))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest29, STest, STest(R"((?=\b))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest30, STest, STest(R"((?!\b))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest31, STest, STest(R"((\b))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest32, STest, STest(R"((?<=\B))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest33, STest, STest(R"((?<!\B))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest34, STest, STest(R"((?=\B))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest35, STest, STest(R"((?!\B))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest36, STest, STest(R"((\B))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest37, STest, STest(R"((?<=^))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest38, STest, STest(R"((?<!^))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest39, STest, STest(R"((?=^))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest40, STest, STest(R"((?!^))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest41, STest, STest(R"((^))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest42, STest, STest(R"((?<=$))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest43, STest, STest(R"((?<!$))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest44, STest, STest(R"((?=$))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest45, STest, STest(R"((?!$))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest46, STest, STest(R"(($))")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest47, STest, STest(R"(\Aa)")) {
  const char text[] = R"(aaa
aaca

a
cac
acc


c
ccca)";
  fixture.search(text, text + 28, 0);
  SCOPE_ASSERT_EQUAL(1u, fixture.Hits.size());
  SCOPE_ASSERT_EQUAL(SearchHit(0, 1, 0), fixture.Hits[0]);
}

// FIXME: Why does this crash?
/*
SCOPE_FIXTURE_CTOR(autoPatternAssertTest48, STest, STest(R"(\Za)")) {
  const char text[] = R"(aaa
aaca

a
cac
acc


c
ccca)";
  fixture.search(text, text + 28, 0);
  SCOPE_ASSERT_EQUAL(0u, fixture.Hits.size());
}
*/

SCOPE_FIXTURE_CTOR(autoPatternAssertTest49, STest, STest(R"(\Ka)")) {
  const char text[] = R"(aaa
aaca

a
cac
acc


c
ccca)";
  fixture.search(text, text + 28, 0);
  SCOPE_ASSERT_EQUAL(10u, fixture.Hits.size());
  SCOPE_ASSERT_EQUAL(SearchHit(0, 1, 0), fixture.Hits[0]);
  SCOPE_ASSERT_EQUAL(SearchHit(1, 2, 0), fixture.Hits[1]);
  SCOPE_ASSERT_EQUAL(SearchHit(2, 3, 0), fixture.Hits[2]);
  SCOPE_ASSERT_EQUAL(SearchHit(4, 5, 0), fixture.Hits[3]);
  SCOPE_ASSERT_EQUAL(SearchHit(5, 6, 0), fixture.Hits[4]);
  SCOPE_ASSERT_EQUAL(SearchHit(7, 8, 0), fixture.Hits[5]);
  SCOPE_ASSERT_EQUAL(SearchHit(10, 11, 0), fixture.Hits[6]);
  SCOPE_ASSERT_EQUAL(SearchHit(13, 14, 0), fixture.Hits[7]);
  SCOPE_ASSERT_EQUAL(SearchHit(16, 17, 0), fixture.Hits[8]);
  SCOPE_ASSERT_EQUAL(SearchHit(27, 28, 0), fixture.Hits[9]);
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest50, STest, STest(R"(\ba)")) {
  const char text[] = R"(aaa
aaca

a
cac
acc


c
ccca)";
  fixture.search(text, text + 28, 0);
  SCOPE_ASSERT_EQUAL(4u, fixture.Hits.size());
  SCOPE_ASSERT_EQUAL(SearchHit(0, 1, 0), fixture.Hits[0]);
  SCOPE_ASSERT_EQUAL(SearchHit(4, 5, 0), fixture.Hits[1]);
  SCOPE_ASSERT_EQUAL(SearchHit(10, 11, 0), fixture.Hits[2]);
  SCOPE_ASSERT_EQUAL(SearchHit(16, 17, 0), fixture.Hits[3]);
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest51, STest, STest(R"(\Ba)")) {
  const char text[] = R"(aaa
aaca

a
cac
acc


c
ccca)";
  fixture.search(text, text + 28, 0);
  SCOPE_ASSERT_EQUAL(6u, fixture.Hits.size());
  SCOPE_ASSERT_EQUAL(SearchHit(1, 2, 0), fixture.Hits[0]);
  SCOPE_ASSERT_EQUAL(SearchHit(2, 3, 0), fixture.Hits[1]);
  SCOPE_ASSERT_EQUAL(SearchHit(5, 6, 0), fixture.Hits[2]);
  SCOPE_ASSERT_EQUAL(SearchHit(7, 8, 0), fixture.Hits[3]);
  SCOPE_ASSERT_EQUAL(SearchHit(13, 14, 0), fixture.Hits[4]);
  SCOPE_ASSERT_EQUAL(SearchHit(27, 28, 0), fixture.Hits[5]);
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest52, STest, STest(R"(^a)")) {
  const char text[] = R"(aaa
aaca

a
cac
acc


c
ccca)";
  fixture.search(text, text + 28, 0);
  SCOPE_ASSERT_EQUAL(4u, fixture.Hits.size());
  SCOPE_ASSERT_EQUAL(SearchHit(0, 1, 0), fixture.Hits[0]);
  SCOPE_ASSERT_EQUAL(SearchHit(4, 5, 0), fixture.Hits[1]);
  SCOPE_ASSERT_EQUAL(SearchHit(10, 11, 0), fixture.Hits[2]);
  SCOPE_ASSERT_EQUAL(SearchHit(16, 17, 0), fixture.Hits[3]);
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest53, STest, STest(R"($a)")) {
  const char text[] = R"(aaa
aaca

a
cac
acc


c
ccca)";
  fixture.search(text, text + 28, 0);
  SCOPE_ASSERT_EQUAL(0u, fixture.Hits.size());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest54, STest, STest(R"(ab)")) {
  const char text[] = R"(aaa
aaca

a
cac
acc


c
ccca)";
  fixture.search(text, text + 28, 0);
  SCOPE_ASSERT_EQUAL(0u, fixture.Hits.size());
}

/*
SCOPE_FIXTURE_CTOR(autoPatternAssertTest55, STest, STest(R"(a\A)")) {
  const char text[] = R"(aaa
aaca

a
cac
acc


c
ccca)";
  fixture.search(text, text + 28, 0);
  SCOPE_ASSERT_EQUAL(0u, fixture.Hits.size());
}
*/

SCOPE_FIXTURE_CTOR(autoPatternAssertTest56, STest, STest(R"(\A\A)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest57, STest, STest(R"(\Z\A)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest58, STest, STest(R"(\K\A)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest59, STest, STest(R"(\b\A)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest60, STest, STest(R"(\B\A)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest61, STest, STest(R"(^\A)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest62, STest, STest(R"($\A)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest63, STest, STest(R"(a\Z)")) {
  const char text[] = R"(aaa
aaca

a
cac
acc


c
ccca)";
  fixture.search(text, text + 28, 0);
  SCOPE_ASSERT_EQUAL(1u, fixture.Hits.size());
  SCOPE_ASSERT_EQUAL(SearchHit(27, 28, 0), fixture.Hits[0]);
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest64, STest, STest(R"(\A\Z)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest65, STest, STest(R"(\Z\Z)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest66, STest, STest(R"(\K\Z)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest67, STest, STest(R"(\b\Z)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest68, STest, STest(R"(\B\Z)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest69, STest, STest(R"(^\Z)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest70, STest, STest(R"($\Z)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest71, STest, STest(R"(a\K)")) {
  const char text[] = R"(aaa
aaca

a
cac
acc


c
ccca)";
  fixture.search(text, text + 28, 0);
  SCOPE_ASSERT_EQUAL(10u, fixture.Hits.size());
  SCOPE_ASSERT_EQUAL(SearchHit(1, 1, 0), fixture.Hits[0]);
  SCOPE_ASSERT_EQUAL(SearchHit(2, 2, 0), fixture.Hits[1]);
  SCOPE_ASSERT_EQUAL(SearchHit(3, 3, 0), fixture.Hits[2]);
  SCOPE_ASSERT_EQUAL(SearchHit(5, 5, 0), fixture.Hits[3]);
  SCOPE_ASSERT_EQUAL(SearchHit(6, 6, 0), fixture.Hits[4]);
  SCOPE_ASSERT_EQUAL(SearchHit(8, 8, 0), fixture.Hits[5]);
  SCOPE_ASSERT_EQUAL(SearchHit(11, 11, 0), fixture.Hits[6]);
  SCOPE_ASSERT_EQUAL(SearchHit(14, 14, 0), fixture.Hits[7]);
  SCOPE_ASSERT_EQUAL(SearchHit(17, 17, 0), fixture.Hits[8]);
  SCOPE_ASSERT_EQUAL(SearchHit(28, 28, 0), fixture.Hits[9]);
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest72, STest, STest(R"(\A\K)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest73, STest, STest(R"(\Z\K)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest74, STest, STest(R"(\K\K)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest75, STest, STest(R"(\b\K)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest76, STest, STest(R"(\B\K)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest77, STest, STest(R"(^\K)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest78, STest, STest(R"($\K)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest79, STest, STest(R"(a\b)")) {
  const char text[] = R"(aaa
aaca

a
cac
acc


c
ccca)";
  fixture.search(text, text + 28, 0);
  SCOPE_ASSERT_EQUAL(4u, fixture.Hits.size());
  SCOPE_ASSERT_EQUAL(SearchHit(2, 3, 0), fixture.Hits[0]);
  SCOPE_ASSERT_EQUAL(SearchHit(7, 8, 0), fixture.Hits[1]);
  SCOPE_ASSERT_EQUAL(SearchHit(10, 11, 0), fixture.Hits[2]);
  SCOPE_ASSERT_EQUAL(SearchHit(27, 28, 0), fixture.Hits[3]);
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest80, STest, STest(R"(\A\b)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest81, STest, STest(R"(\Z\b)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest82, STest, STest(R"(\K\b)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest83, STest, STest(R"(\b\b)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest84, STest, STest(R"(\B\b)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest85, STest, STest(R"(^\b)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest86, STest, STest(R"($\b)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest87, STest, STest(R"(a\B)")) {
  const char text[] = R"(aaa
aaca

a
cac
acc


c
ccca)";
  fixture.search(text, text + 28, 0);
  SCOPE_ASSERT_EQUAL(6u, fixture.Hits.size());
  SCOPE_ASSERT_EQUAL(SearchHit(0, 1, 0), fixture.Hits[0]);
  SCOPE_ASSERT_EQUAL(SearchHit(1, 2, 0), fixture.Hits[1]);
  SCOPE_ASSERT_EQUAL(SearchHit(4, 5, 0), fixture.Hits[2]);
  SCOPE_ASSERT_EQUAL(SearchHit(5, 6, 0), fixture.Hits[3]);
  SCOPE_ASSERT_EQUAL(SearchHit(13, 14, 0), fixture.Hits[4]);
  SCOPE_ASSERT_EQUAL(SearchHit(16, 17, 0), fixture.Hits[5]);
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest88, STest, STest(R"(\A\B)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest89, STest, STest(R"(\Z\B)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest90, STest, STest(R"(\K\B)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest91, STest, STest(R"(\b\B)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest92, STest, STest(R"(\B\B)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest93, STest, STest(R"(^\B)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest94, STest, STest(R"($\B)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest95, STest, STest(R"(a^)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest96, STest, STest(R"(\A^)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest97, STest, STest(R"(\Z^)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest98, STest, STest(R"(\K^)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest99, STest, STest(R"(\b^)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest100, STest, STest(R"(\B^)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest101, STest, STest(R"(^^)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest102, STest, STest(R"($^)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest103, STest, STest(R"(a$)")) {
  const char text[] = R"(aaa
aaca

a
cac
acc


c
ccca)";
  fixture.search(text, text + 28, 0);
  SCOPE_ASSERT_EQUAL(4u, fixture.Hits.size());
  SCOPE_ASSERT_EQUAL(SearchHit(2, 3, 0), fixture.Hits[0]);
  SCOPE_ASSERT_EQUAL(SearchHit(7, 8, 0), fixture.Hits[1]);
  SCOPE_ASSERT_EQUAL(SearchHit(10, 11, 0), fixture.Hits[2]);
  SCOPE_ASSERT_EQUAL(SearchHit(27, 28, 0), fixture.Hits[3]);
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest104, STest, STest(R"(\A$)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest105, STest, STest(R"(\Z$)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest106, STest, STest(R"(\K$)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest107, STest, STest(R"(\b$)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest108, STest, STest(R"(\B$)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest109, STest, STest(R"(^$)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest110, STest, STest(R"($$)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest111, STest, STest(R"(\A*)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest112, STest, STest(R"(\Z*)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest113, STest, STest(R"(\K*)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest114, STest, STest(R"(\b*)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest115, STest, STest(R"(\B*)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest116, STest, STest(R"(^*)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest117, STest, STest(R"($*)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest118, STest, STest(R"(\A|a)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest119, STest, STest(R"(\Z|a)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest120, STest, STest(R"(\K|a)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest121, STest, STest(R"(\b|a)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest122, STest, STest(R"(\B|a)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest123, STest, STest(R"(^|a)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest124, STest, STest(R"($|a)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest125, STest, STest(R"(a|b)")) {
  const char text[] = R"(aaa
aaca

a
cac
acc


c
ccca)";
  fixture.search(text, text + 28, 0);
  SCOPE_ASSERT_EQUAL(10u, fixture.Hits.size());
  SCOPE_ASSERT_EQUAL(SearchHit(0, 1, 0), fixture.Hits[0]);
  SCOPE_ASSERT_EQUAL(SearchHit(1, 2, 0), fixture.Hits[1]);
  SCOPE_ASSERT_EQUAL(SearchHit(2, 3, 0), fixture.Hits[2]);
  SCOPE_ASSERT_EQUAL(SearchHit(4, 5, 0), fixture.Hits[3]);
  SCOPE_ASSERT_EQUAL(SearchHit(5, 6, 0), fixture.Hits[4]);
  SCOPE_ASSERT_EQUAL(SearchHit(7, 8, 0), fixture.Hits[5]);
  SCOPE_ASSERT_EQUAL(SearchHit(10, 11, 0), fixture.Hits[6]);
  SCOPE_ASSERT_EQUAL(SearchHit(13, 14, 0), fixture.Hits[7]);
  SCOPE_ASSERT_EQUAL(SearchHit(16, 17, 0), fixture.Hits[8]);
  SCOPE_ASSERT_EQUAL(SearchHit(27, 28, 0), fixture.Hits[9]);
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest126, STest, STest(R"(a|\A)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest127, STest, STest(R"(\A|\A)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest128, STest, STest(R"(\Z|\A)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest129, STest, STest(R"(\K|\A)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest130, STest, STest(R"(\b|\A)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest131, STest, STest(R"(\B|\A)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest132, STest, STest(R"(^|\A)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest133, STest, STest(R"($|\A)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest134, STest, STest(R"(a|\Z)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest135, STest, STest(R"(\A|\Z)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest136, STest, STest(R"(\Z|\Z)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest137, STest, STest(R"(\K|\Z)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest138, STest, STest(R"(\b|\Z)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest139, STest, STest(R"(\B|\Z)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest140, STest, STest(R"(^|\Z)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest141, STest, STest(R"($|\Z)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest142, STest, STest(R"(a|\K)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest143, STest, STest(R"(\A|\K)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest144, STest, STest(R"(\Z|\K)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest145, STest, STest(R"(\K|\K)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest146, STest, STest(R"(\b|\K)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest147, STest, STest(R"(\B|\K)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest148, STest, STest(R"(^|\K)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest149, STest, STest(R"($|\K)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest150, STest, STest(R"(a|\b)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest151, STest, STest(R"(\A|\b)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest152, STest, STest(R"(\Z|\b)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest153, STest, STest(R"(\K|\b)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest154, STest, STest(R"(\b|\b)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest155, STest, STest(R"(\B|\b)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest156, STest, STest(R"(^|\b)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest157, STest, STest(R"($|\b)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest158, STest, STest(R"(a|\B)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest159, STest, STest(R"(\A|\B)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest160, STest, STest(R"(\Z|\B)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest161, STest, STest(R"(\K|\B)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest162, STest, STest(R"(\b|\B)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest163, STest, STest(R"(\B|\B)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest164, STest, STest(R"(^|\B)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest165, STest, STest(R"($|\B)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest166, STest, STest(R"(a|^)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest167, STest, STest(R"(\A|^)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest168, STest, STest(R"(\Z|^)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest169, STest, STest(R"(\K|^)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest170, STest, STest(R"(\b|^)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest171, STest, STest(R"(\B|^)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest172, STest, STest(R"(^|^)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest173, STest, STest(R"($|^)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest174, STest, STest(R"(a|$)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest175, STest, STest(R"(\A|$)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest176, STest, STest(R"(\Z|$)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest177, STest, STest(R"(\K|$)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest178, STest, STest(R"(\b|$)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest179, STest, STest(R"(\B|$)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest180, STest, STest(R"(^|$)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}

SCOPE_FIXTURE_CTOR(autoPatternAssertTest181, STest, STest(R"($|$)")) {
  SCOPE_ASSERT(fixture.parsesButNotValid());
}
