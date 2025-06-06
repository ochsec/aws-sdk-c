include(AwsTestHarness)
enable_testing()

file(GLOB TEST_SRC "*.c")
file(GLOB TEST_HDRS "*.h")
file(GLOB TESTS ${TEST_HDRS} ${TEST_SRC})

add_test_case(sdkutils_library_test)

add_test_case(aws_profile_early_property_parse_failure_test)
add_test_case(aws_profile_missing_bracket_parse_failure_test)
add_test_case(aws_profile_missing_assignment_parse_failure_test)
add_test_case(aws_profile_missing_property_key_parse_failure_test)
add_test_case(aws_profile_early_continuation_parse_failure_test)
add_test_case(aws_profile_illegal_continuation1_parse_failure_test)
add_test_case(aws_profile_illegal_continuation2_parse_failure_test)
add_test_case(aws_profile_illegal_continuation3_parse_failure_test)
add_test_case(aws_profile_continuation_reset_on_new_profile_parse_failure_test)

add_test_case(aws_profile_empty_test)
add_test_case(aws_profile_empty_profile_test)
add_test_case(aws_profile_whitespace_empty_profile_test)
add_test_case(aws_profile_tab_empty_profile_test)
add_test_case(aws_profile_single_simple_property_profile_test)
add_test_case(aws_profile_equal_containing_property_profile_test)
add_test_case(aws_profile_unicode_containing_property_profile_test)
add_test_case(aws_profile_multiple_property_profile_test)
add_test_case(aws_profile_trimmable_property_profile_test)
add_test_case(aws_profile_empty_property_profile_test)
add_test_case(aws_profile_multiple_empty_profile_test)
add_test_case(aws_profile_multiple_profile_test)
add_test_case(aws_profile_multiple_sections)
add_test_case(aws_profile_sections_in_credentials_test)
add_test_case(aws_profile_sso_session_without_name_test)
add_test_case(aws_profile_services_invalid_prefix_test)
add_test_case(aws_profile_blank_lines_ignored_test)
add_test_case(aws_profile_pound_comments_ignored_test)
add_test_case(aws_profile_semicolon_comments_ignored_test)
add_test_case(aws_profile_mixed_comments_ignored_test)
add_test_case(aws_profile_empty_comments_ignored_test)
add_test_case(aws_profile_profile_adjacent_comment_test)
add_test_case(aws_profile_value_adjacent_comment_test)
add_test_case(aws_profile_continued_property_value_test)
add_test_case(aws_profile_multiline_continued_property_value_test)
add_test_case(aws_profile_continued_property_value_trim_test)
add_test_case(aws_profile_continued_property_value_pound_comment_test)
add_test_case(aws_profile_continued_property_value_semicolon_comment_test)
add_test_case(aws_profile_duplicate_profiles_merge_test)
add_test_case(aws_profile_duplicate_properties_last_property_value_test)
add_test_case(aws_profile_duplicate_profiles_last_property_value_test)
add_test_case(aws_profile_duplicate_default_profiles_property_resolution1_test)
add_test_case(aws_profile_duplicate_default_profiles_property_resolution2_test)
add_test_case(aws_profile_invalid_profile_names_merge_test)
add_test_case(aws_profile_invalid_property_names_ignored_test)
add_test_case(aws_profile_all_valid_profile_characters_test)
add_test_case(aws_profile_all_valid_property_characters_test)
add_test_case(aws_profile_basic_sub_property_test)
add_test_case(aws_profile_empty_sub_property_test)
add_test_case(aws_profile_invalid_sub_property_name_test)
add_test_case(aws_profile_sub_property_blank_line_test)
add_test_case(aws_profile_basic_duplicate_merge_test)
add_test_case(aws_profile_mixed_prefix_default_test)
add_test_case(aws_profile_override_duplicate_merge_test)
add_test_case(aws_profile_no_prefix_nondefault_test)
add_test_case(aws_profile_prefix_credentials_test)

add_test_case(parse_resource_name_test)
add_test_case(parse_resource_name_failures_test)
add_test_case(resource_name_tostring_test)
add_test_case(resource_name_tostring_failure_test)
add_test_case(resource_name_length_test)

add_test_case(parse_ruleset_from_string)
add_test_case(test_endpoints_aws_region)
add_test_case(test_endpoints_default_values)
add_test_case(test_endpoints_eventbridge)
add_test_case(test_endpoints_fns)
add_test_case(test_endpoints_get_attr_type_inference)
add_test_case(test_endpoints_headers)
add_test_case(test_endpoints_is_virtual_hostable_s3_bucket)
add_test_case(test_endpoints_region_override)
add_test_case(test_endpoints_minimal_ruleset)
add_test_case(test_endpoints_parse_arn)
add_test_case(test_endpoints_parse_url)
add_test_case(test_endpoints_partition_fn)
add_test_case(test_endpoints_substring)
add_test_case(test_endpoints_uri_encode)
add_test_case(test_endpoints_valid_hostlabel)
add_test_case(test_endpoints_condition_mem_clean_up)
add_test_case(test_endpoints_custom)
add_test_case(test_endpoints_string_array)

add_test_case(endpoints_uri_normalize_path)
add_test_case(endpoints_byte_buf_init_from_resolved_templated_string)

add_test_case(endpoints_regex_aws_region_matches)
add_test_case(endpoints_regex_iso_region_matches)
add_test_case(endpoints_regex_misc_validation)

set(TEST_BINARY_NAME ${PROJECT_NAME}-tests)
generate_test_driver(${TEST_BINARY_NAME})

add_custom_command(TARGET ${PROJECT_NAME}-tests PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${PROJECT_NAME}-tests>)
