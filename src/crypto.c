#include "../include/crypto.h"

/**
 * @brief Internal function - Set the counter buffer
 *
 * @param counter Counter to set.
 * @param counter_buf Buffer to set.
 */
void set_counter_buffer(unsigned int counter, unsigned char *counter_buf)
{
        for (int i = 7; i >= 0; i--) {
                counter_buf[i] = counter & 0xff;
                counter >>= 8;
        }
}

/**
 * @brief Internal function - Set the algorithm to use
 *
 * @param algo Algorithm to use (e.g. HMAC_SHA1 or HMAC_SHA256).
 * @return int Return Code (0 success, otherwise error)
 */
static int _set_algo(const char *algo)
{
        if (strcmp(algo, HMAC_SHA1) == 0) {
            return 20; // SHA-1 produces 20-byte hashes
        } else if (strcmp(algo, HMAC_SHA256) == 0) {
            return 32; // SHA-256 produces 32-byte hashes
        } else {
            printk(KERN_ERR "Unsupported algorithm\n");
            return -EINVAL;
        }
}

/**
 * @brief Internal function - Allocate the HMAC transform
 *
 * @param tfm Structure that manages the transform.
 * @param algo Algorithm to use. (e.g. HMAC_SHA1 or HMAC_SHA256)
 * @return int Return Code (0 success, otherwise error)
 */
static int _allocate_hmac_transform(struct crypto_ahash **tfm, const char *algo)
{
        *tfm = crypto_alloc_ahash(algo, 0, 0);

        if (IS_ERR(*tfm)) {
            printk(KERN_ERR "Failed to allocate hmac transform\n");
            return PTR_ERR(*tfm);
        }
        return 0;
}

/**
 * @brief Internal function - Set the key for the HMAC transform
 *
 * @param tfm Structure that manages the transform.
 * @param key Key to set.
 * @param len Length of the key.
 * @return int Return Code (0 success, otherwise error)
 */
static int _set_hmac_key(struct crypto_ahash **tfm, const char *key, size_t len)
{
        int ret;

        ret = crypto_ahash_setkey(*tfm, key, len);
        if (ret) {
                printk(KERN_ERR "Failed to set key: %d\n", ret);
                crypto_free_ahash(*tfm);
                return ret;
        }
        return 0;
}

/**
 * @brief Internal function - Allocate the request for the HMAC transform
 *
 * @param req Structure that manages the request.
 * @param tfm Structure that manages the transform.
 * @return int Return Code (0 success, otherwise error)
 */
static int _request_hmac_alloc(struct ahash_request **req, struct crypto_ahash *tfm)
{
        *req = ahash_request_alloc(tfm, GFP_KERNEL);
        if (!*req) {
                printk(KERN_ERR "Failed to allocate ahash request\n");
                crypto_free_ahash(tfm);
                return -ENOMEM;
        }
        return 0;
}

/**
 * @brief Internal function - Manage the request for the HMAC transform
 *
 * @param req Structure that manages the request.
 * @param sg Structure that manages the scatterlist.
 * @param message Message to digest.
 * @param len Length of the message.
 * @param output Output buffer.
 */
static void _manage_hmac_request(struct ahash_request *req, struct scatterlist *sg, const char *message, size_t len, unsigned char *output)
{
        sg_init_one(sg, message, len);

        ahash_request_set_callback(req, 0, NULL, NULL);
        ahash_request_set_crypt(req, sg, output, len);
}

/**
 * @brief Internal function - Digest the request for the HMAC transform
 *
 * @param req Structure that manages the request.
 * @param output Output buffer.
 * @return int Return Code (0 success, otherwise error)
 */
static int _digest_hmac_request(struct ahash_request *req, unsigned char *output)
{
        int ret = crypto_ahash_digest(req);

        if (ret) {
            printk(KERN_ERR "HMAC computation failed: %d\n", ret);
            return ret;
        }

        printk(KERN_INFO "HMAC: ");
        for (int i = 0; i < 32; i++)
                printk(KERN_CONT "%02x", output[i]);
        printk(KERN_CONT "\n");
        return 0;
}

/**
 * @brief Internal function - Convert the hash to a code
 *
 * @param hash Hash to convert.
 * @return uint32_t Code generated from the hash.
 */
static unsigned int _hash_to_code(const unsigned char *hash, size_t hash_len)
{
        int offset = hash[hash_len-1] & 0xf;
        uint32_t binary = ((hash[offset] & 0x7f) << 24) |
                          ((hash[offset + 1] & 0xff) << 16) |
                          ((hash[offset + 2] & 0xff) << 8) |
                          (hash[offset + 3] & 0xff);
        int code = binary % int_pow(10, 6); // Default to 6 digits

        printk(KERN_INFO "HOTP Code: %06d\n", code);
        return (code);
}

/**
 * @brief HOTP Algorithm
 *
 * @param algo Algorithm to use (e.g. HMAC_SHA1 or HMAC_SHA256).
 * @param key Key to use.
 * @param message Message to digest.
 * @return int Return Code (0 success, otherwise error)
 */
int hotp_algo(const char *algo, const char *key, unsigned char *message)
{
        if (!algo || !key || !message) {
                printk(KERN_ERR "Invalid arguments to hotp_algo\n");
                return -EINVAL;
        }

        size_t len = strlen(message);
        size_t key_len = strlen(key);
        int algo_len = _set_algo(algo);
        if (algo_len <= 0) {
                printk(KERN_ERR "Invalid algorithm length\n");
                return -EINVAL;
        }

        unsigned char *hotp_output = kmalloc(algo_len, GFP_KERNEL);
        if (!hotp_output) {
                printk(KERN_ERR "Failed to allocate memory for HMAC output\n");
                return -ENOMEM;
        }

        struct crypto_ahash *tfm = NULL;
        struct ahash_request *req = NULL;
        struct scatterlist sg;
        int code = 0;

        // Allocate HMAC transform
        if (_allocate_hmac_transform(&tfm, algo) != 0) {
                printk(KERN_ERR "Failed to allocate HMAC transform\n");
                kfree(hotp_output);
                return -EINVAL;
        }

        // Set HMAC key
        if (_set_hmac_key(&tfm, key, key_len) != 0) {
                printk(KERN_ERR "Failed to set HMAC key\n");
                crypto_free_ahash(tfm);
                kfree(hotp_output);
                return -EINVAL;
        }

        // Allocate HMAC request
        if (_request_hmac_alloc(&req, tfm) != 0) {
                printk(KERN_ERR "Failed to allocate HMAC request\n");
                crypto_free_ahash(tfm);
                kfree(hotp_output);
                return -ENOMEM;
        }

        // Manage and digest HMAC request
        _manage_hmac_request(req, &sg, message, len, hotp_output);
        if (_digest_hmac_request(req, hotp_output) != 0) {
                printk(KERN_ERR "Failed to digest HMAC request\n");
                ahash_request_free(req);
                crypto_free_ahash(tfm);
                kfree(hotp_output);
                return -EFAULT;
        }

        // Convert hash to OTP code
        code = _hash_to_code(hotp_output, algo_len);

        // Clean up
        ahash_request_free(req);
        crypto_free_ahash(tfm);
        kfree(hotp_output);

        return code;
}
