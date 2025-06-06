include(AwsLibFuzzer)
include(AwsTestHarness)
enable_testing()

file(GLOB TEST_HDRS "*.h")
file(GLOB TEST_SRC "*.c")

file(GLOB TESTS ${TEST_HDRS} ${TEST_SRC})

add_test_case(sha256_nist_test_case_1)
add_test_case(sha256_nist_test_case_2)
add_test_case(sha256_nist_test_case_3)
add_test_case(sha256_nist_test_case_4)
add_test_case(sha256_nist_test_case_5)
add_test_case(sha256_nist_test_case_5_truncated)
add_test_case(sha256_nist_test_case_6)
add_test_case(sha256_test_invalid_buffer)
add_test_case(sha256_test_oneshot)
add_test_case(sha256_test_invalid_state)
add_test_case(sha256_test_extra_buffer_space)

add_test_case(sha1_nist_test_case_1)
add_test_case(sha1_nist_test_case_2)
add_test_case(sha1_nist_test_case_3)
add_test_case(sha1_nist_test_case_4)
add_test_case(sha1_nist_test_case_5)
add_test_case(sha1_nist_test_case_5_truncated)
add_test_case(sha1_nist_test_case_6)
add_test_case(sha1_test_invalid_buffer)
add_test_case(sha1_test_oneshot)
add_test_case(sha1_test_invalid_state)
add_test_case(sha1_test_extra_buffer_space)

add_test_case(md5_rfc1321_test_case_1)
add_test_case(md5_rfc1321_test_case_2)
add_test_case(md5_rfc1321_test_case_3)
add_test_case(md5_rfc1321_test_case_4)
add_test_case(md5_rfc1321_test_case_5)
add_test_case(md5_rfc1321_test_case_6)
add_test_case(md5_rfc1321_test_case_7)
add_test_case(md5_rfc1321_test_case_7_truncated)
add_test_case(md5_verify_known_collision)
add_test_case(md5_invalid_buffer_size)
add_test_case(md5_test_invalid_state)
add_test_case(md5_test_extra_buffer_space)

add_test_case(sha256_hmac_rfc4231_test_case_1)
add_test_case(sha256_hmac_rfc4231_test_case_2)
add_test_case(sha256_hmac_rfc4231_test_case_3)
add_test_case(sha256_hmac_rfc4231_test_case_4)
add_test_case(sha256_hmac_rfc4231_test_case_5)
add_test_case(sha256_hmac_rfc4231_test_case_6)
add_test_case(sha256_hmac_rfc4231_test_case_7)
add_test_case(sha256_hmac_test_oneshot)
add_test_case(sha256_hmac_test_invalid_buffer)
add_test_case(sha256_hmac_test_invalid_state)
add_test_case(sha256_hmac_test_extra_buffer_space)

add_test_case(ecdsa_p256_test_pub_key_derivation)
add_test_case(ecdsa_p384_test_pub_key_derivation)
add_test_case(ecdsa_p256_test_known_signing_value)
add_test_case(ecdsa_p384_test_known_signing_value)
add_test_case(ecdsa_test_invalid_signature)
add_test_case(ecdsa_p256_test_key_gen)
add_test_case(ecdsa_p384_test_key_gen)
add_test_case(ecdsa_p256_test_key_gen_export)
add_test_case(ecdsa_p384_test_key_gen_export)
add_test_case(ecdsa_p256_test_import_asn1_key_pair)
add_test_case(ecdsa_p384_test_import_asn1_key_pair)
add_test_case(ecdsa_test_import_asn1_key_pair_public_only)
add_test_case(ecdsa_test_import_asn1_key_pair_invalid_fails)
add_test_case(ecdsa_test_signature_format)
add_test_case(ecdsa_p256_test_small_coordinate_verification)

add_test_case(rsa_encryption_roundtrip_pkcs1_from_user)
add_test_case(rsa_encryption_roundtrip_oaep_sha256_from_user)
add_test_case(rsa_encryption_roundtrip_oaep_sha512_from_user)
add_test_case(rsa_signing_roundtrip_pkcs1_sha256_from_user)
add_test_case(rsa_signing_roundtrip_pss_sha256_from_user)
add_test_case(rsa_signing_roundtrip_pkcs1_sha1_from_user)
add_test_case(rsa_getters)
add_test_case(rsa_private_pkcs1_der_parsing)
add_test_case(rsa_public_pkcs1_der_parsing)
add_test_case(rsa_verify_signing_pkcs1_sha256)
add_test_case(rsa_verify_signing_pkcs1_sha1)
add_test_case(rsa_verify_signing_pss_sha256)
add_test_case(rsa_decrypt_pkcs1)
add_test_case(rsa_decrypt_oaep256)
add_test_case(rsa_decrypt_oaep512)
add_test_case(rsa_signing_mismatch_pkcs1_sha256)
add_test_case(rsa_signing_mismatch_pkcs1_sha1)
add_test_case(rsa_encryption_roundtrip_pkcs15_from_user_pkcs8)

add_test_case(aes_cbc_NIST_CBCGFSbox256_case_1)
add_test_case(aes_cbc_NIST_CBCVarKey256_case_254)
add_test_case(aes_cbc_NIST_CBCVarTxt256_case_110)
add_test_case(aes_cbc_NIST_CBCMMT256_case_4)
add_test_case(aes_cbc_NIST_CBCMMT256_case_9)
add_test_case(aes_cbc_test_with_generated_key_iv)
add_test_case(aes_cbc_validate_materials_fails)
add_test_case(aes_ctr_RFC3686_Case_7)
add_test_case(aes_ctr_RFC3686_Case_8)
add_test_case(aes_ctr_RFC3686_Case_9)
add_test_case(aes_ctr_test_with_generated_key_iv)
add_test_case(aes_ctr_validate_materials_fails)
add_test_case(gcm_NIST_gcmEncryptExtIV256_PTLen_128_Test_0)
add_test_case(gcm_NIST_gcmEncryptExtIV256_PTLen_104_Test_3)
add_test_case(gcm_NIST_gcmEncryptExtIV256_PTLen_256_Test_6)
add_test_case(gcm_NIST_gcmEncryptExtIV256_PTLen_408_Test_8)
add_test_case(gcm_256_KAT_1)
add_test_case(gcm_256_KAT_2)
add_test_case(gcm_256_KAT_3)
add_test_case(gcm_test_with_generated_key_iv)
add_test_case(aes_gcm_validate_materials_fails)
add_test_case(aes_keywrap_RFC3394_256BitKey256CekTestVector)
add_test_case(aes_keywrap_Rfc3394_256BitKey_TestIntegrityCheckFailed)
add_test_case(aes_keywrap_RFC3394_256BitKeyTestBadPayload)
add_test_case(aes_keywrap_RFC3394_256BitKey128BitCekTestVector)
add_test_case(aes_keywrap_RFC3394_256BitKey128BitCekIntegrityCheckFailedTestVector)
add_test_case(aes_keywrap_RFC3394_256BitKey128BitCekPayloadCheckFailedTestVector)
add_test_case(aes_keywrap_validate_materials_fails)
add_test_case(aes_test_input_too_large)
add_test_case(aes_test_encrypt_empty_input)
add_test_case(aes_test_empty_input_gcm_tag_corner_cases)
add_test_case(aes_test_gcm_tag_corner_cases)
add_test_case(aes_test_gcm_tag_large_input_corner_cases)

add_test_case(der_encode_integer)
add_test_case(der_encode_integer_zero)
add_test_case(der_encode_boolean)
add_test_case(der_encode_null)
add_test_case(der_encode_bit_string)
add_test_case(der_encode_octet_string)
add_test_case(der_encode_sequence)
add_test_case(der_encode_set)
add_test_case(der_decode_negative_int)
add_test_case(der_decode_positive_int)
add_test_case(der_decode_zero_int)
add_test_case(der_decode_bad_length)
add_test_case(der_decode_zero_length_int)

add_test_case(der_decode_integer)
add_test_case(der_decode_integer_zero)
add_test_case(der_decode_boolean)
add_test_case(der_decode_null)
add_test_case(der_decode_bit_string)
add_test_case(der_decode_octet_string)
add_test_case(der_decode_sequence)
add_test_case(der_decode_set)
add_test_case(der_decode_key_pair)

add_test_case(ecc_key_pair_random_ref_count_test)
add_test_case(ecc_key_pair_public_ref_count_test)
add_test_case(ecc_key_pair_asn1_ref_count_test)
add_test_case(ecc_key_pair_private_ref_count_test)
add_test_case(ecc_key_gen_from_private_fuzz_test)

add_test_case(ed25519_key_pair_generate_test)

generate_test_driver(${PROJECT_NAME}-tests)

# OpenBSD 7.4+ defaults to linking with --execute-only which is not always safe for AWS-LC.
# We have similar link flags in bindings, but in this case we need on the test executable,
# because ed25519 keygen is hitting the same issue 
# (according to aws-lc huge tables in text section for ed25519 is the likely culprit)
if(CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
    target_link_options(${PROJECT_NAME}-tests PRIVATE "-Wl,--no-execute-only")
endif()
