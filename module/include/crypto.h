/*
** EPITECH PROJECT, 2025
** OTP
** File description:
** crypto.h
** Header file listing all the functions usable in crypto.c
*/
#ifndef CRYPTO_H
        #define CRYPTO_H

        #include <linux/printk.h>
        #include <crypto/hash.h>
        #include <linux/crypto.h>
        #include <linux/scatterlist.h>
        #include <linux/err.h>

        #define HMAC_SHA1 "hmac(sha1)"
        #define HMAC_SHA256 "hmac(sha256)"
        #define SECRET_MAX_LEN 64

        extern int counter; //Counter for algo pseudo randomness

        /**
         * @brief HOTP Algorithm
         *
         * @param algo Algorithm to use (e.g. HMAC_SHA1 or HMAC_SHA256).
         * @param key Key to use.
         * @param message Message to digest.
         * @return int Return Code (0 success, otherwise error)
         */
        int hotp_algo(const char *algo, const char *key, unsigned char *message);

        /**
         * @brief Internal function - Set the counter buffer
         *
         * @param counter Counter to set.
         * @param counter_buf Buffer to set.
         */
        void set_counter_buffer(unsigned int counter, unsigned char *counter_buf);
#endif /* !CRYPTO_H_ */