How do I derive a key for an encrypted OpenSSH private key?
============================================================

OpenSSH ``ssh-keygen`` encrypts new-format private keys
(``-----BEGIN OPENSSH PRIVATE KEY-----``) using a symmetric cipher (typically
``aes256-ctr``) keyed by ``bcrypt_pbkdf``. To decrypt a key produced by
``ssh-keygen``, derive the cipher key plus IV using the salt and round count
embedded in the key file:

.. code-block:: cpp

   #include <polycpp/bcrypt_pbkdf.hpp>
   #include <polycpp/buffer.hpp>

   // From the OpenSSH private-key blob:
   //   kdf == "bcrypt"
   //   kdfoptions == { salt: <16 bytes>, rounds: 16 }
   //   ciphername == "aes256-ctr"  -> 32-byte key + 16-byte IV = 48 bytes
   polycpp::Buffer derive_aes256_ctr_key_iv(const polycpp::Buffer& password,
                                             const polycpp::Buffer& salt,
                                             std::uint32_t rounds) {
       return polycpp::bcrypt_pbkdf::pbkdf(password, salt, rounds, /*keylen=*/48);
   }

The first 32 bytes of the returned ``Buffer`` are the AES-256 key; the
remaining 16 bytes are the AES-CTR IV. Pass those into your AES decryption
primitive (for example ``polycpp::crypto::createDecipheriv("aes-256-ctr",
key, iv)``) to decrypt the key blob.

.. note::

   ``rounds`` for ``ssh-keygen``-produced keys is typically 16 (the default
   is 16 since OpenSSH 9.5). Some older keys use 24. Always read the value
   from the key file rather than hard-coding it.

.. note::

   The salt length in OpenSSH keys is 16 bytes. ``bcrypt_pbkdf`` itself
   accepts up to 1 MiB; the public OpenSSH format uses exactly 16.
