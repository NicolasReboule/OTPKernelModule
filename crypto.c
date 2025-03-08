#include <crypto/hash.h>
#include <linux/crypto.h>
#include <linux/scatterlist.h>

#include <linux/err.h>
#include <linux/printk.h>

#define HMAC_SHA1_ALGO "hmac(sha1)"
#define HMAC_SHA256_ALGO "hmac(sha256)"

static unsigned char hotp_output[20];

/**
 * @brief Internal function - Allocate the HMAC transform
 *
 * @param tfm Structure that manages the transform.
 * @param algo Algorithm to use. (e.g. HMAC_SHA1_ALGO or HMAC_SHA256_ALGO /!\ Change the output buffer size /!\)
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
static int _set_key(struct crypto_ahash **tfm, const char *key, size_t len)
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
static int _set_request_alloc(struct ahash_request **req, struct crypto_ahash *tfm)
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
static void _manage_request(struct ahash_request *req, struct scatterlist *sg, const char *message, size_t len, unsigned char *output)
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
static int _digest_request(struct ahash_request *req, unsigned char *output)
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
 * @brief HOTP Algorithm
 *
 * @param key Key to use.
 * @param message Message to digest.
 * @return int Return Code (0 success, otherwise error)
 */
static int hotp_algo(const char *key, const char *message)
{
        size_t key_len = strlen(key);
        size_t len = strlen(message);
        int ret;

        struct crypto_ahash *tfm;
        struct ahash_request *req;
        struct scatterlist sg;

        if ((ret = _allocate_hmac_transform(&tfm, HMAC_SHA256_ALGO)) != 0) return ret;
        if ((ret = _set_key(&tfm, key, key_len)) != 0) return ret;
        if ((ret = _set_request_alloc(&req, tfm)) != 0) return ret;

        _manage_request(req, &sg, message, len, hotp_output);
        _digest_request(req, hotp_output);

        ahash_request_free(req);
        crypto_free_ahash(tfm);
        return 0;
}