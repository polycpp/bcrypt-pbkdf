// Tests for fail-closed parameter validation.
//
// Each test confirms that an invalid input throws PbkdfError with the
// expected PbkdfErrorCode before any keying material is computed, mirroring
// upstream's `return -1;` checks at the top of bcrypt_pbkdf.

#include <polycpp/bcrypt_pbkdf.hpp>
#include <polycpp/buffer.hpp>
#include <polycpp/core/error.hpp>

#include <gtest/gtest.h>

#include <cstring>
#include <exception>
#include <string>
#include <vector>

using polycpp::bcrypt_pbkdf::PbkdfError;
using polycpp::bcrypt_pbkdf::PbkdfErrorCode;

TEST(Errors, ZeroRoundsThrowsInvalidRounds) {
    auto password = polycpp::Buffer::from("password");
    auto salt = polycpp::Buffer::from("salt");
    try {
        (void)polycpp::bcrypt_pbkdf::pbkdf(password, salt, 0, 32);
        FAIL() << "expected PbkdfError";
    } catch (const PbkdfError& e) {
        EXPECT_EQ(e.code(), PbkdfErrorCode::InvalidRounds);
    }
}

TEST(Errors, EmptyPasswordThrowsEmptyPassword) {
    auto password = polycpp::Buffer::alloc(0);
    auto salt = polycpp::Buffer::from("salt");
    try {
        (void)polycpp::bcrypt_pbkdf::pbkdf(password, salt, 4, 32);
        FAIL() << "expected PbkdfError";
    } catch (const PbkdfError& e) {
        EXPECT_EQ(e.code(), PbkdfErrorCode::EmptyPassword);
    }
}

TEST(Errors, EmptySaltThrowsEmptySalt) {
    auto password = polycpp::Buffer::from("password");
    auto salt = polycpp::Buffer::alloc(0);
    try {
        (void)polycpp::bcrypt_pbkdf::pbkdf(password, salt, 4, 32);
        FAIL() << "expected PbkdfError";
    } catch (const PbkdfError& e) {
        EXPECT_EQ(e.code(), PbkdfErrorCode::EmptySalt);
    }
}

TEST(Errors, ZeroKeylenThrowsEmptyKeylen) {
    auto password = polycpp::Buffer::from("password");
    auto salt = polycpp::Buffer::from("salt");
    try {
        (void)polycpp::bcrypt_pbkdf::pbkdf(password, salt, 4, 0);
        FAIL() << "expected PbkdfError";
    } catch (const PbkdfError& e) {
        EXPECT_EQ(e.code(), PbkdfErrorCode::EmptyKeylen);
    }
}

TEST(Errors, KeylenAt1024IsAccepted) {
    // Boundary case: upstream rejects keylen > HASHSIZE * HASHSIZE = 1024,
    // so 1024 itself is valid. Run with rounds=1 to keep the test fast.
    auto password = polycpp::Buffer::from("password");
    auto salt = polycpp::Buffer::from("salt");
    auto key = polycpp::bcrypt_pbkdf::pbkdf(password, salt, 1, 1024);
    EXPECT_EQ(key.length(), 1024u);
}

TEST(Errors, KeylenAbove1024ThrowsKeylenTooLarge) {
    auto password = polycpp::Buffer::from("password");
    auto salt = polycpp::Buffer::from("salt");
    try {
        (void)polycpp::bcrypt_pbkdf::pbkdf(password, salt, 4, 1025);
        FAIL() << "expected PbkdfError";
    } catch (const PbkdfError& e) {
        EXPECT_EQ(e.code(), PbkdfErrorCode::KeylenTooLarge);
    }
}

TEST(Errors, OversizeSaltThrowsSaltTooLarge) {
    auto password = polycpp::Buffer::from("password");
    // 1 << 20 + 1 = 1048577 bytes — one byte over the maximum.
    polycpp::Buffer big_salt = polycpp::Buffer::alloc((std::size_t{1} << 20) + 1);
    try {
        (void)polycpp::bcrypt_pbkdf::pbkdf(password, big_salt, 4, 32);
        FAIL() << "expected PbkdfError";
    } catch (const PbkdfError& e) {
        EXPECT_EQ(e.code(), PbkdfErrorCode::SaltTooLarge);
    }
}

TEST(Errors, BcryptHashRejectsShortInput) {
    polycpp::Buffer short_input = polycpp::Buffer::alloc(63);
    polycpp::Buffer ok_input = polycpp::Buffer::alloc(64);
    try {
        (void)polycpp::bcrypt_pbkdf::bcryptHash(short_input, ok_input);
        FAIL() << "expected PbkdfError";
    } catch (const PbkdfError& e) {
        EXPECT_EQ(e.code(), PbkdfErrorCode::InvalidHashInput);
    }
}

TEST(Errors, BcryptHashRejectsLongInput) {
    polycpp::Buffer ok_input = polycpp::Buffer::alloc(64);
    polycpp::Buffer long_input = polycpp::Buffer::alloc(65);
    try {
        (void)polycpp::bcrypt_pbkdf::bcryptHash(ok_input, long_input);
        FAIL() << "expected PbkdfError";
    } catch (const PbkdfError& e) {
        EXPECT_EQ(e.code(), PbkdfErrorCode::InvalidHashInput);
    }
}

TEST(Errors, ErrorCodeNamesAreReadable) {
    EXPECT_STREQ(polycpp::bcrypt_pbkdf::pbkdfErrorCodeName(PbkdfErrorCode::InvalidRounds),
                 "InvalidRounds");
    EXPECT_STREQ(polycpp::bcrypt_pbkdf::pbkdfErrorCodeName(PbkdfErrorCode::KeylenTooLarge),
                 "KeylenTooLarge");
}

// Exception inheritance: PbkdfError is catchable as polycpp::Error and as
// std::exception. This is part of the API contract — generic logging code
// that catches std::exception must see PbkdfError.
TEST(Errors, PbkdfErrorIsCatchableAsPolycppError) {
    auto password = polycpp::Buffer::from("password");
    auto salt = polycpp::Buffer::from("salt");
    bool caught = false;
    try {
        (void)polycpp::bcrypt_pbkdf::pbkdf(password, salt, 0, 32);
    } catch (const polycpp::Error& e) {
        caught = true;
        EXPECT_NE(std::string(e.what()).find("rounds"), std::string::npos);
    }
    EXPECT_TRUE(caught);
}

TEST(Errors, PbkdfErrorIsCatchableAsStdException) {
    auto password = polycpp::Buffer::from("password");
    auto salt = polycpp::Buffer::from("salt");
    bool caught = false;
    try {
        (void)polycpp::bcrypt_pbkdf::pbkdf(password, salt, 4, 0);
    } catch (const std::exception& e) {
        caught = true;
        EXPECT_NE(std::string(e.what()).find("keylen"), std::string::npos);
    }
    EXPECT_TRUE(caught);
}

// Every PbkdfErrorCode has a non-empty message — useful for log lines and
// guards against silent enum additions without a string mapping.
TEST(Errors, EveryCodeHasReadableName) {
    constexpr PbkdfErrorCode codes[] = {
        PbkdfErrorCode::InvalidRounds,
        PbkdfErrorCode::EmptyPassword,
        PbkdfErrorCode::EmptySalt,
        PbkdfErrorCode::EmptyKeylen,
        PbkdfErrorCode::KeylenTooLarge,
        PbkdfErrorCode::SaltTooLarge,
        PbkdfErrorCode::InvalidHashInput,
    };
    for (auto code : codes) {
        const char* name = polycpp::bcrypt_pbkdf::pbkdfErrorCodeName(code);
        ASSERT_NE(name, nullptr);
        EXPECT_GT(std::strlen(name), 0u);
        EXPECT_STRNE(name, "Unknown");
    }
}
