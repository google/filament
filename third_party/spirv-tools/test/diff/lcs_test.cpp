// Copyright (c) 2022 Google LLC.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "source/diff/lcs.h"

#include <string>

#include "gtest/gtest.h"

namespace spvtools {
namespace diff {
namespace {

using Sequence = std::vector<int>;
using LCS = LongestCommonSubsequence<Sequence>;

void VerifyMatch(const Sequence& src, const Sequence& dst,
                 size_t expected_match_count) {
  DiffMatch src_match, dst_match;

  LCS lcs(src, dst);
  size_t match_count =
      lcs.Get<int>([](int s, int d) { return s == d; }, &src_match, &dst_match);

  EXPECT_EQ(match_count, expected_match_count);

  size_t src_cur = 0;
  size_t dst_cur = 0;
  size_t matches_seen = 0;

  while (src_cur < src.size() && dst_cur < dst.size()) {
    if (src_match[src_cur] && dst_match[dst_cur]) {
      EXPECT_EQ(src[src_cur], dst[dst_cur])
          << "Src: " << src_cur << " Dst: " << dst_cur;
      ++src_cur;
      ++dst_cur;
      ++matches_seen;
      continue;
    }
    if (!src_match[src_cur]) {
      ++src_cur;
    }
    if (!dst_match[dst_cur]) {
      ++dst_cur;
    }
  }

  EXPECT_EQ(matches_seen, expected_match_count);
}

TEST(LCSTest, EmptySequences) {
  Sequence src, dst;

  DiffMatch src_match, dst_match;

  LCS lcs(src, dst);
  size_t match_count =
      lcs.Get<int>([](int s, int d) { return s == d; }, &src_match, &dst_match);

  EXPECT_EQ(match_count, 0u);
  EXPECT_TRUE(src_match.empty());
  EXPECT_TRUE(dst_match.empty());
}

TEST(LCSTest, EmptySrc) {
  Sequence src, dst = {1, 2, 3};

  DiffMatch src_match, dst_match;

  LCS lcs(src, dst);
  size_t match_count =
      lcs.Get<int>([](int s, int d) { return s == d; }, &src_match, &dst_match);

  EXPECT_EQ(match_count, 0u);
  EXPECT_TRUE(src_match.empty());
  EXPECT_EQ(dst_match, DiffMatch(3, false));
}

TEST(LCSTest, EmptyDst) {
  Sequence src = {1, 2, 3}, dst;

  DiffMatch src_match, dst_match;

  LCS lcs(src, dst);
  size_t match_count =
      lcs.Get<int>([](int s, int d) { return s == d; }, &src_match, &dst_match);

  EXPECT_EQ(match_count, 0u);
  EXPECT_EQ(src_match, DiffMatch(3, false));
  EXPECT_TRUE(dst_match.empty());
}

TEST(LCSTest, Identical) {
  Sequence src = {1, 2, 3, 4, 5, 6}, dst = {1, 2, 3, 4, 5, 6};

  DiffMatch src_match, dst_match;

  LCS lcs(src, dst);
  size_t match_count =
      lcs.Get<int>([](int s, int d) { return s == d; }, &src_match, &dst_match);

  EXPECT_EQ(match_count, 6u);
  EXPECT_EQ(src_match, DiffMatch(6, true));
  EXPECT_EQ(dst_match, DiffMatch(6, true));
}

TEST(LCSTest, SrcPrefix) {
  Sequence src = {1, 2, 3, 4}, dst = {1, 2, 3, 4, 5, 6};

  DiffMatch src_match, dst_match;

  LCS lcs(src, dst);
  size_t match_count =
      lcs.Get<int>([](int s, int d) { return s == d; }, &src_match, &dst_match);

  const DiffMatch src_expect = {true, true, true, true};
  const DiffMatch dst_expect = {true, true, true, true, false, false};

  EXPECT_EQ(match_count, 4u);
  EXPECT_EQ(src_match, src_expect);
  EXPECT_EQ(dst_match, dst_expect);
}

TEST(LCSTest, DstPrefix) {
  Sequence src = {1, 2, 3, 4, 5, 6}, dst = {1, 2, 3, 4, 5};

  DiffMatch src_match, dst_match;

  LCS lcs(src, dst);
  size_t match_count =
      lcs.Get<int>([](int s, int d) { return s == d; }, &src_match, &dst_match);

  const DiffMatch src_expect = {true, true, true, true, true, false};
  const DiffMatch dst_expect = {true, true, true, true, true};

  EXPECT_EQ(match_count, 5u);
  EXPECT_EQ(src_match, src_expect);
  EXPECT_EQ(dst_match, dst_expect);
}

TEST(LCSTest, SrcSuffix) {
  Sequence src = {3, 4, 5, 6}, dst = {1, 2, 3, 4, 5, 6};

  DiffMatch src_match, dst_match;

  LCS lcs(src, dst);
  size_t match_count =
      lcs.Get<int>([](int s, int d) { return s == d; }, &src_match, &dst_match);

  const DiffMatch src_expect = {true, true, true, true};
  const DiffMatch dst_expect = {false, false, true, true, true, true};

  EXPECT_EQ(match_count, 4u);
  EXPECT_EQ(src_match, src_expect);
  EXPECT_EQ(dst_match, dst_expect);
}

TEST(LCSTest, DstSuffix) {
  Sequence src = {1, 2, 3, 4, 5, 6}, dst = {5, 6};

  DiffMatch src_match, dst_match;

  LCS lcs(src, dst);
  size_t match_count =
      lcs.Get<int>([](int s, int d) { return s == d; }, &src_match, &dst_match);

  const DiffMatch src_expect = {false, false, false, false, true, true};
  const DiffMatch dst_expect = {true, true};

  EXPECT_EQ(match_count, 2u);
  EXPECT_EQ(src_match, src_expect);
  EXPECT_EQ(dst_match, dst_expect);
}

TEST(LCSTest, None) {
  Sequence src = {1, 3, 5, 7, 9}, dst = {2, 4, 6, 8, 10, 12};

  DiffMatch src_match, dst_match;

  LCS lcs(src, dst);
  size_t match_count =
      lcs.Get<int>([](int s, int d) { return s == d; }, &src_match, &dst_match);

  EXPECT_EQ(match_count, 0u);
  EXPECT_EQ(src_match, DiffMatch(5, false));
  EXPECT_EQ(dst_match, DiffMatch(6, false));
}

TEST(LCSTest, NonContiguous) {
  Sequence src = {1, 2, 3, 4, 5, 6, 10}, dst = {2, 4, 5, 8, 9, 10, 12};

  DiffMatch src_match, dst_match;

  LCS lcs(src, dst);
  size_t match_count =
      lcs.Get<int>([](int s, int d) { return s == d; }, &src_match, &dst_match);

  const DiffMatch src_expect = {false, true, false, true, true, false, true};
  const DiffMatch dst_expect = {true, true, true, false, false, true, false};

  EXPECT_EQ(match_count, 4u);
  EXPECT_EQ(src_match, src_expect);
  EXPECT_EQ(dst_match, dst_expect);
}

TEST(LCSTest, WithDuplicates) {
  Sequence src = {1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4},
           dst = {1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4};
  VerifyMatch(src, dst, 6);
}

TEST(LCSTest, Large) {
  const std::string src_str =
      "GUJwrJSlkKJXxCVIAxlVgnUyOrdyRyFtlZwWMmFhYGfkFTNnhiBmClgHyrcXMVwfrRxNUfQk"
      "qaoGvCbPZHAzXsaZpXHPfJxOMCUtRDmIQpfiXKbHQbhTfPqhxBDWvmTQAqwsWTLajZYtMUnf"
      "hNNCfkuAXkZsaebwEbIZOxTDZsqSMUfCMoGeKJGVSNFgLTiBMbdvchHGfFRkHKcYCDjBfIcj"
      "todPnvDjzQYWBcvIfVvyBzHikrwpDORaGEZLhmyztIFCLJOqeLhOzERYmVqzlsoUzruTXTXq"
      "DLTxQRakOCMRrgRzCDTXfwwfDcKMBVnxRZemjcwcEsOVxwtwdBCWJycsDcZKlvrCvZaenKlv"
      "vyByDQeLdxAyBnPkIMQlMQwqjUfRLybeoaOanlbFkpTPPZdHelQrIvucTHzMpWWQTbuANwvN"
      "OVhCGGoIcGNDpfIsaBexlMMdHsxMGerTngmjpdPeQQJHfvKZkdYqAzrtDohqtDsaFMxQViVQ"
      "YszDVgyoSHZdOXAvXkJidojLvGZOzhRajVPhWDwKuGqdaympELxHsrXAJYufdCPwJdGJfWqq"
      "yvTWpcrFHOIuCEmNLnSCDsxQGRVDwyCykBJazhApfCnrOadnafvqfVuFqEXMSrYbHTfTnzbz"
      "MhISyOtMUITaurCXvanCbuOXBhHyCjOhVbxnMvhlPmZBMQgEHCghtAJVMXGPNRtszVZlPxVl"
      "QIPTBnPUPejlyZGPqeICyNngdQGkvKbIoWlTLBtVhMdBeUMozNlKQTIPYBeImVcMdLafuxUf"
      "TIXysmcTrUTcspOSKBxhdhLwiRnREGFWJTfUKsgGOAQeYojXdrqsGjMJfiKalyoiqrgLnlij"
      "CtOapoxDGVOOBalNYGzCtBlxbvaAzxipGnJpOEbmXcpeoIsAdxspKBzBDgoPVxnuRBUwmTSr"
      "CRpWhxikgUYQVCwalLUIeBPRyhhsECGCXJmGDZSCIUaBwROkigzdeVPOXhgCGBEprWtNdYfL"
      "tOUYJHQXxiIJgSGmWntezJFpNQoTPbRRYAGhtYvAechvBcYWocLkYFxsDAuszvQNLXdhmAHw"
      "DErcjbtCdQllnKcDADVNWVezljjLrAuyGHetINMgAvJZwOEYakihYVUbZGCsHEufluLNyNHy"
      "gqtSTSFFjBHiIqQejTPWybLdpWNwZrWvIWnlzUcGNQPEYHVPCbteWknjAnWrdTBeCbHUDBoK"
      "aHvDStmpNRGIjvlumiZTbdZNAzUeSFnFChCsSExwXeEfDJfjyOoSBofHzJqJErvHLNyUJTjX"
      "qmtgKPpMKohUPBMhtCteQFcNEpWrUVGbibMOpvBwdiWYXNissArpSasVJFgDzrqTyGkerTMX"
      "gcrzFUGFZRhNdekaJeKYPogsofJaRsUQmIRyYdkrxKeMgLPpwOfSKJOqzXDoeHljTzhOwEVy"
      "krOEnACFrWhufajsMitjOWdLOHHchQDddGPzxknEgdwmZepKDvRZGCuPqzeQkjOPqUBKpKLJ"
      "eKieSsRXkaqxSPGajfvPKmwFWdLByEcLgvrmteazgFjmMGrLYqRRxzUOfOCokenqHVYstBHf"
      "AwsWsqPTvqsRJUfGGTaYiylZMGbQqTzINhFHvdlRQvvYKBcuAHdBeKlHSxVrSsEKbcAvnIcf"
      "xzdVDdwQPHMCHeZZRpGHWvKzgTGzSTbYTeOPyKvvYWmQToTpsjAtKUJUjcEHWhmdBLDTBMHJ"
      "ivBXcLGtCsumNNVFyGbVviGmqHTdyBlkneibXBesKJGOUzOtIwXCPJggqBekSzNQYkALlItk"
      "cbEhbdXAIKVHYpInLwxXalKZrkrpxtfuagqMGmRJnJbFQaEoYMoqPsxZpocddPXXPyvxVkaF"
      "qdKISejWDhBImnEEOPDcyWTubbfVfwUztciaFJcsPLhgYVfhqlOfoNjKbmTFptFttYuyBrUI"
      "zzmZypOqrjQHTGFwlHStpIwxPtMvtsEDpsmWIgwzYgwmdpbMOnfElZMYpVIcvzSWejeJcdUB"
      "QUoBRUmGQVVWvEDseuozrDjgdXFScPwwsgaUPwSzScfBNrkpmEFDSZLKfNjMqvOmUtocUkbo"
      "VGFEKgGLbNruwLgXHTloWDrnqymPVAtzjWPutonIsMDPeeCmTjYWAFXcyTAlBeiJTIRkZxiM"
      "kLjMnAflSNJzmZkatXkYiPEMYSmzHbLKEizHbEjQOxBDzpRHiFjhedqiyMiUMvThjaRFmwll"
      "aMGgwKBIKepwyoEdnuhtzJzboiNEAFKiqiWxxmkRFRoTiFWXLPAWLuzSCrajgkQhDxAQDqyM"
      "VwZlhZicQLEDYYisEalesDWZAYzcvENuHUwRutIsGgsdoYwOZiURhcgdbTGWBNqhrFjvTQCj"
      "VlTPNlRdRLaaqzUBBwbdtyXFkCBUYYMbmRrkFxfxbCqkgZNGyHPKLkOPnezfVTRmRQgCgHbx"
      "wcZlInVOwmFePnSIbThMJosimzkhfuiqYEpwHQiemqsSDNNdbNhBLzbsPZBJZujSHJGtYKGb"
      "HaAYGJZxBumsKUrATwPuqXFLfwNyImLQbchBKiJAYRZhkcrKCHXBEGYyBhBGvSqvabcRUrfq"
      "AbPiMzjHAehGYjDEmxAnYLyoSFdeWVrfJUCuYZPluhXEBuyUpKaRXDKXeiCvGidpvATwMbcz"
      "DZpzxrhTZYyrFORFQWTbPLCBjMKMhlRMFEiarDgGPttjmkrQVlujztMSkxXffXFNqLWOLThI"
      "KBoyMHoFTEPCdUAZjLTifAdjjUehyDLEGKlRTFoLpjalziRSUjZfRYbNzhiHgTHowMMkKTwE"
      "ZgnqiirMtnNpaBJqhcIVrWXPpcPWZfRpsPstHleFJDZYAsxYhOREVbFtebXTZRAIjGgWeoiN"
      "qPLCCAVadqmUrjOcqIbdCTpcDRWuDVbHrZOQRPhqbyvOWwxAWJphjLiDgoAybcjzgfVktPlj"
      "kNBCjelpuQfnYsiTgPpCNKYtOrxGaLEEtAuLdGdDsONHNhSn";
  const std::string dst_str =
      "KzitfifORCbGhfNEbnbObUdFLLaAsLOpMkOeKupjCoatzqfHBkNJfSgqSMYouswfNMnoQngK"
      "jWwyPKmEnoZWyPBUdQRmKUNudUclueKXKQefUdXWUyyqtumzsFKznrLVLwfvPZpLChNYrrHK"
      "AtpfOuVHiUKyeRCrktJAhkyFKmPWrASEMvBLNOzuGlvinZjvZUUXazNEkyMPiOLdqXvCIroC"
      "MeWsvjHShlLhDwLZrVlpYBnDJmILcsNFDSoaLWOKNNkNGBgNBvVjPCJXAuKfsrKZhYcdEpxK"
      "UihiRkYvMiLyOUvaqBMklLDwEhvQBfCXHSRoqsLsSCzLZQhIYMhBapvHaPbDoRrHoJXZsNXc"
      "rxZYCrOMIzYcVPwDCFiHBFnPNTTeAeKEMGeVUeCaAeuWZmngyPWlQBcgWumSUIfbhjVYdnpV"
      "hRSJXrIoFZubBXfNOMhilAkVPixrhILZKgDoFTvytPFPfBLMnbhSOBmLWCbJsLQxrCrMAlOw"
      "RmfSQyGhrjhzYVqFSBHeoQBagFwyxIjcHFZngntpVHbSwqhwHeMnWSsISPljTxSNXfCxLebW"
      "GhMdlphtJbdvhEcjNpwPCFqhdquxCyOxkjsDUPNgjpDcpIMhMwMclNhfESTrroJaoyeGQclV"
      "gonnhuQRmXcBwcsWeLqjNngZOlyMyfeQBwnwMVJEvGqknDyzSApniRTPgJpFoDkJJhXQFuFB"
      "VqhuEPMRGCeTDOSEFmXeIHOnDxaJacvnmORwVpmrRhGjDpUCkuODNPdZMdupYExDEDnDLdNF"
      "iObKBaVWpGVMKdgNLgsNxcpypBPPKKoaajeSGPZQJWSOKrkLjiFexYVmUGxJnbTNsCXXLfZp"
      "jfxQAEVYvqKehBzMsVHVGWmTshWFAoCNDkNppzzjHBZWckrzSTANICioCJSpLwPwQvtXVxst"
      "nTRBAboPFREEUFazibpFesCsjzUOnECwoPCOFiwGORlIZVLpUkJyhYXCENmzTBLVigOFuCWO"
      "IiXBYmiMtsxnUdoqSTTGyEFFrQsNAjcDdOKDtHwlANWoUVwiJCMCQFILdGqzEePuSXFbOEOz"
      "dLlEnTJbKRSTfAFToOZNtDXTfFgvQiefAKbSUWUXFcpCjRYCBNXCCcLMjjuUDXErpiNsRuIx"
      "mgHsrObTEXcnmjdqxTGhTjTeYizNnkrJRhNQIqDXmZMwArBccnixpcuiGOOexjgkpcEyGAnz"
      "UbgiBfflTUyJfZeFFLrZVueFkSRosebnnwAnakIrywTGByhQKWvmNQJsWQezqLhHQzXnEpeD"
      "rFRTSQSpVxPzSeEzfWYzfpcenxsUyzOMLxhNEhfcuprDtqubsXehuqKqZlLQeSclvoGjuKJK"
      "XoWrazsgjXXnkWHdqFESZdMGDYldyYdbpSZcgBPgEKLWZHfBirNPLUadmajYkiEzmGuWGELB"
      "WLiSrMdaGSbptKmgYVqMGcQaaATStiZYteGAPxSEBHuAzzjlRHYsrdDkaGNXmzRGoalJMiCC"
      "GMtWSDMhgvRSEgKnywbRgnqWXFlwrhXbbvcgLGtWSuKQBiqIlWkfPMozOTWgVoLHavDJGRYI"
      "YerrmZnTMtuuxmZALWakfzUbksTwoetqkOiRPGqGZepcVXHoZyOaaaijjZWQLlIhYwiQNbfc"
      "KCwhhFaMQBoaCnOecJEdKzdsMPFEYQuJNPYiiNtsYxaWBRuWjlLqGokHMNtyTQfSJKbgGdol"
      "fWlOZdupouQMfUWXIYHzyJHefMDnqxxasDxtgArvDqtwjDBaVEMACPkLFpiDOoKCHqkWVizh"
      "lKqbOHpsPKkhjRQRNGYRYEfxtBjYvlCvHBNUwVuIwDJYMqHxEFtwdLqYWvjdOfQmNiviDfUq"
      "pbucbNwjNQfMYgwUuPnQWIPOlqHcbjtuDXvTzLtkdBQanJbrmLSyFqSapZCSPMDOrxWVYzyO"
      "lwDTTJFmKxoyfPunadkHcrcSQaQsAbrQtbhqwSTXGTPURYTCbNozjAVwbmcyVxIbZudBZWYm"
      "rnSDyelGCRRWYtrUxvOVWlTLHHdYuAmVMGnGbHscbjmjmAzmYLaCxNNwhmMYdExKvySxuYpE"
      "rVGwfqMngBCHnZodotNaNJZiNRFWubuPDfiywXPiyVWoQMeOlSuWmpilLTIFOvfpjmJTgrWa"
      "dgoxYeyPyOaglOvZVGdFOBSeqEcGXBwjoeUAXqkpvOxEpSXhmklKZydTvRVYVvfQdRNNDkCT"
      "dLNfcZCFQbZORdcDOhwotoyccrSbWvlqYMoiAYeEpDzZTvkamapzZMmCpEutZFCcHBWGIIkr"
      "urwDNHrobaErPpclyEegLJDtkfUWSNWZosWSbBGAHIvJsFNUlJXbnkSVycLkOVQVcNcUtiBy"
      "djLDIFsycbPBEWaMvCbntNtJlOeCttvXypGnHAQFnFSiXFWWqonWuVIKmVPpKXuJtFguXCWC"
      "rNExYYvxLGEmuZJLJDjHgjlQyOzeieCpizJxkrdqKCgomyEkvsyVYSsLeyLvOZQrrgEJgRFK"
      "CjYtoOfluNrLdRMTRkQXmAiMRFwloYECpXCReAMxOkNiwCtutsrqWoMHsrogRqPoUCueonvW"
      "MTwmkAkajfGJkhnQidwpwIMEttQkzIMOPvvyWZHpqkMHWlNTeSKibfRfwDyxveKENZhtlPwP"
      "dfAjwegjRcavtFnkkTNVYdCdCrgdUvzsIcqmUjwGmVvuuQvjVrWWIDBmAzQtiZPYvCOEWjce"
      "rWzeqVKeiYTJBOedmQCVidOgUIEjfRnbGvUbctYxfRybJkdmeAkLZQMRMGPOnsPbFswXAoCK"
      "IxWGwohoPpEJxslbqHFKSwknxTmrDCITRZWEDkGQeucPxHBdYkduwbYhKnoxCKhgjBFiFawC"
      "QtgTDldTQmlOsBiGLquMjuecAbrUJJvNtXbFNGjWxaZPimSRXUJWgRbydpsczOqSFIeEtuKA"
      "ZpRhmLtPdVNKdSDQZeeImUFmUwXApRTUNHItyvFyJtNtn";

  Sequence src;
  Sequence dst;

  src.reserve(src_str.length());
  dst.reserve(dst_str.length());

  for (char c : src_str) {
    src.push_back(c);
  }
  for (char c : dst_str) {
    dst.push_back(c);
  }

  VerifyMatch(src, dst, 723);
}

}  // namespace
}  // namespace diff
}  // namespace spvtools
