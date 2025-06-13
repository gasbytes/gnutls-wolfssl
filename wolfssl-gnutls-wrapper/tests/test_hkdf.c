
#include <gnutls/crypto.h>

#include "test_util.h"


const unsigned char ikm_data[] = {
    0x7b, 0xf1, 0x05, 0x31, 0x36, 0xfa, 0x03, 0xdc,
    0x31, 0x97, 0x88, 0x04, 0x9c, 0xbc, 0xee, 0xf7,
    0x8d, 0x84, 0x95, 0x26, 0xaf, 0x1d, 0x68, 0xb0,
    0x60, 0x7a, 0xcc, 0x4f, 0xc1, 0xd3, 0xa1, 0x68,
    0x7f, 0x6d, 0xbe
};
const unsigned char salt_data[] = {
    0x28, 0x00, 0xf9, 0x65, 0x00, 0xf9, 0x8d, 0xba,
    0x58, 0x18, 0x65, 0x60, 0xa3, 0x4a, 0xbe, 0xb8,
    0xef, 0x58, 0xb0, 0x0b, 0xab, 0xf2, 0xae, 0x99,
    0x44, 0x91, 0x78, 0x8e, 0x8b, 0x8e, 0xeb, 0x39,
};
const char* info_data = "tls13 ";

const unsigned char expected_prk_sha256[] = {
    0x8d, 0x02, 0x24, 0xc2, 0x51, 0x23, 0x01, 0xe3,
    0x7f, 0x0c, 0x06, 0x4f, 0x68, 0xa4, 0xef, 0xfa,
    0x29, 0xe4, 0xca, 0xe2, 0x63, 0x32, 0x54, 0x9b,
    0xce, 0x71, 0xae, 0x6a, 0xea, 0xb8, 0xbf, 0x5c
};
const unsigned char expected_okm_sha256[] = {
    0xb5, 0x4e, 0x54, 0xd4, 0x3a, 0xaf, 0x8e, 0xe8,
    0x5c, 0x40, 0xc8, 0x08, 0x4f, 0x8b, 0x47, 0x96,
    0xd3, 0xf2, 0xb5, 0x8f, 0xce, 0x08, 0xce, 0x48,
    0x27, 0x43, 0x16, 0xa5, 0x45, 0xd5, 0x08, 0xa0
};
const unsigned char expected_prk_sha384[] = {
    0xfd, 0x2f, 0x03, 0xc0, 0xdb, 0x57, 0x47, 0x06,
    0xf0, 0x42, 0xf9, 0x9a, 0x4b, 0x0f, 0x02, 0xd3,
    0x32, 0x38, 0xe5, 0x81, 0x95, 0x03, 0xb1, 0x02,
    0xe6, 0x64, 0xb1, 0x8f, 0x8c, 0x61, 0x42, 0xba,
    0x82, 0x1e, 0x50, 0x1c, 0x4b, 0x0d, 0xa5, 0xd4,
    0x0b, 0xa4, 0x6d, 0x95, 0x72, 0x74, 0xb7, 0x09
};
const unsigned char expected_okm_sha384[] = {
    0x01, 0x24, 0x6d, 0xb2, 0xe8, 0x3a, 0x49, 0xfa,
    0x24, 0xee, 0x0c, 0x54, 0x2e, 0xeb, 0xeb, 0xd1,
    0x8f, 0xf5, 0x7d, 0x4c, 0xd3, 0x94, 0x20, 0x9d,
    0x15, 0xfb, 0x01, 0x3d, 0x97, 0xa4, 0xad, 0x48,
    0xbe, 0x79, 0x66, 0xbd, 0x59, 0x91, 0x01, 0x05,
    0xb4, 0x5f, 0x6f, 0x27, 0xf7, 0x7b, 0xab, 0xeb
};


static int test_hkdf(gnutls_mac_algorithm_t mac_alg,
    const unsigned char* expected_prk, size_t prk_sz,
    const unsigned char* expected_okm, size_t okm_sz)
{
    int ret;
    unsigned char prk_data[64];
    unsigned char okm_data[64];
    gnutls_datum_t ikm = {
        .data = (unsigned char*)ikm_data,
        .size = sizeof(ikm_data)
    };
    gnutls_datum_t salt = {
        .data = (unsigned char*)salt_data,
        .size = sizeof(salt_data)
    };
    gnutls_datum_t info = {
        .data = (unsigned char*)info_data,
        .size = strlen(info_data)
    };
    gnutls_datum_t prk = {
        .data = prk_data,
        .size = prk_sz,
    };

    /* Rest of test code remains the same */
    printf("Testing wolfSSL's %s HKDF implementation via GnuTLS...\n",
        gnutls_mac_get_name(mac_alg));

    ret = gnutls_hkdf_extract(mac_alg, &ikm, &salt, prk_data);
    if (ret != 0) {
        print_gnutls_error("calculating master secret", ret);
        return 1;
    }

    if (compare("PRK", prk_data, expected_prk, prk_sz) != 0) {
        return 1;
    }

    ret = gnutls_hkdf_expand(mac_alg, &prk, &info, okm_data, okm_sz);
    if (ret != 0) {
        print_gnutls_error("calculating master secret", ret);
        return 1;
    }

    if (compare("PRK", okm_data, expected_okm, okm_sz) != 0) {
        return 1;
    }

    return 0;
}

int main(void)
{
    int ret;

    /* Initialize GnuTLS */
    ret = gnutls_global_init();
    if (ret != 0) {
        print_gnutls_error("initializing GnuTLS", ret);
        return 1;
    }

    ret = test_hkdf(GNUTLS_MAC_SHA256, expected_prk_sha256,
        sizeof(expected_prk_sha256), expected_okm_sha256,
        sizeof(expected_okm_sha256));
    if (ret == 0) {
        ret = test_hkdf(GNUTLS_MAC_SHA384, expected_prk_sha384,
            sizeof(expected_prk_sha384), expected_okm_sha384,
            sizeof(expected_okm_sha384));
    }
    if (ret == 0) {
        printf("Test completed.\n");
    }

    gnutls_global_deinit();

    return ret;
}

