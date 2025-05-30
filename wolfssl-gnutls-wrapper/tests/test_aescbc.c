
#include <gnutls/crypto.h>

#include "test_util.h"


/* Test vectors from NIST SP 800-38A, F.2.1 CBC-AES128.Encrypt */
const unsigned char key_128[] = {
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};
const unsigned char key_192[] = {
    0x3e, 0x41, 0xf1, 0x17, 0xdc, 0x9e, 0x0f, 0x01,
    0x9b, 0x58, 0x96, 0x4d, 0x29, 0xe3, 0xfe, 0x6b,
    0x60, 0x95, 0x81, 0x02, 0xcc, 0xef, 0xfe, 0x6e,
};
const unsigned char key_256[] = {
    0x00, 0xa9, 0x9e, 0x4e, 0x75, 0x84, 0x69, 0xc5,
    0x5c, 0xb4, 0xb8, 0xf8, 0xb9, 0x48, 0x7f, 0xde,
    0x7d, 0xd2, 0xbd, 0x5a, 0xb2, 0x45, 0xee, 0x38,
    0x3f, 0x4d, 0x82, 0x0e, 0xe1, 0x46, 0xd9, 0x23,
};

const unsigned char iv_data[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};

const unsigned char plaintext_data[] = {
    0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
    0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
    0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
    0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51
};

const unsigned char expected_ciphertext_128[] = {
    0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46,
    0xce, 0xe9, 0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d,
    0x50, 0x86, 0xcb, 0x9b, 0x50, 0x72, 0x19, 0xee,
    0x95, 0xdb, 0x11, 0x3a, 0x91, 0x76, 0x78, 0xb2
};
const unsigned char expected_ciphertext_192[] = {
    0x24, 0xc1, 0x64, 0x62, 0x9e, 0x28, 0x03, 0xa1,
    0xc4, 0x4b, 0xfa, 0xd9, 0x0a, 0x3f, 0x81, 0x52,
    0x92, 0xd9, 0xbe, 0xd8, 0x3f, 0xe6, 0x57, 0xa1,
    0xd9, 0x9f, 0x18, 0x22, 0xbf, 0x68, 0xe8, 0xc3
};
const unsigned char expected_ciphertext_256[] = {
    0xb6, 0x14, 0x9f, 0x15, 0xe4, 0x3a, 0x67, 0x47,
    0xbe, 0xb9, 0xc8, 0xde, 0x0a, 0x87, 0x67, 0xff,
    0x85, 0xbc, 0x4d, 0x68, 0x38, 0x47, 0x5e, 0xf8,
    0x14, 0xd9, 0xe7, 0xa3, 0x6b, 0xb5, 0xf4, 0xd5
};


static int test_aescbc(gnutls_cipher_algorithm_t cipher,
    const unsigned char* key_data, size_t key_data_sz,
    const unsigned char* expected_ciphertext, size_t expected_ciphertext_sz)
{
    int ret;
    unsigned char ciphertext[sizeof(plaintext_data)];
    unsigned char decrypted[sizeof(plaintext_data)];
    unsigned char plaintext[sizeof(plaintext_data)];

    /* Create gnutls_datum_t structures for key and IV */
    gnutls_datum_t key = {
        .data = (unsigned char *)key_data,
        .size = key_data_sz
    };

    gnutls_datum_t iv = {
        .data = (unsigned char *)iv_data,
        .size = sizeof(iv_data)
    };

    /* Create a fresh IV for decryption (GnuTLS modifies the IV during
     * operation) */
    gnutls_datum_t decrypt_iv = {
        .data = (unsigned char *)iv_data,
        .size = sizeof(iv_data)
    };

    /* Copy plaintext to a non-const buffer for GnuTLS */
    memcpy(plaintext, plaintext_data, sizeof(plaintext_data));

    /********** ENCRYPTION TEST **********/
    /* Encrypt using AES-128-CBC */
    gnutls_cipher_hd_t encrypt_handle;
    if ((ret = gnutls_cipher_init(&encrypt_handle, cipher, &key, &iv)) < 0) {
        print_gnutls_error("initializing cipher for encryption", ret);
        return 1;
    }

    if ((ret = gnutls_cipher_encrypt(encrypt_handle, plaintext,
            sizeof(plaintext))) < 0) {
        print_gnutls_error("encrypting", ret);
        gnutls_cipher_deinit(encrypt_handle);
        return 1;
    }

    /* Copy the encrypted data to our ciphertext buffer */
    memcpy(ciphertext, plaintext, sizeof(plaintext));

    gnutls_cipher_deinit(encrypt_handle);

    if (compare("Encryption", ciphertext, expected_ciphertext,
            sizeof(ciphertext)) != 0) {
        return 1;
    }

    /********** DECRYPTION TEST **********/
    /* Decrypt using AES-128-CBC */
    gnutls_cipher_hd_t decrypt_handle;
    if ((ret = gnutls_cipher_init(&decrypt_handle, cipher, &key,
            &decrypt_iv)) < 0) {
        print_gnutls_error("initializing cipher for decryption", ret);
        return 1;
    }

    /* Copy ciphertext to a buffer for decryption */
    memcpy(decrypted, ciphertext, sizeof(ciphertext));

    if ((ret = gnutls_cipher_decrypt(decrypt_handle, decrypted,
            sizeof(decrypted))) < 0) {
        print_gnutls_error("decrypting", ret);
        gnutls_cipher_deinit(decrypt_handle);
        return 1;
    }

    gnutls_cipher_deinit(decrypt_handle);

    if (compare("Decryption", decrypted, plaintext_data,
            sizeof(decrypted)) != 0) {
        return 1;
    }

    return 0;
}

int main(void)
{
    int ret;

    /* Initialize GnuTLS */
    if ((ret = gnutls_global_init()) < 0) {
        print_gnutls_error("initializing GnuTLS", ret);
        return 1;
    }

    ret = test_aescbc(GNUTLS_CIPHER_AES_128_CBC, key_128, sizeof(key_128),
        expected_ciphertext_128, sizeof(expected_ciphertext_128));
    if (ret == 0) {
        ret = test_aescbc(GNUTLS_CIPHER_AES_192_CBC, key_192, sizeof(key_192),
            expected_ciphertext_192, sizeof(expected_ciphertext_192));
    }
    if (ret == 0) {
        ret = test_aescbc(GNUTLS_CIPHER_AES_256_CBC, key_256, sizeof(key_256),
            expected_ciphertext_256, sizeof(expected_ciphertext_256));
    }
    if (ret == 0) {
        printf("Test completed.\n");
    }

    gnutls_global_deinit();

    return ret;
}

