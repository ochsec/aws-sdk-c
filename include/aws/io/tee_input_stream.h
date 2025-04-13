/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#ifndef AWS_IO_TEE_INPUT_STREAM_H
#define AWS_IO_TEE_INPUT_STREAM_H

#include <aws/common/common.h>
#include <aws/io/stream.h>

AWS_EXTERN_C_BEGIN

/**
 * @brief Creates a new tee input stream that allows multiple independent reads 
 * from a source stream by buffering its entire content.
 *
 * @param allocator Memory allocator to use
 * @param source The source input stream to wrap
 * @return struct aws_input_stream* Pointer to the new tee input stream, or NULL on failure
 */
AWS_IO_API struct aws_input_stream *aws_tee_input_stream_create(
    struct aws_allocator *allocator,
    struct aws_input_stream *source);

/**
 * @brief Creates a new branch stream from an existing tee input stream.
 *
 * @param tee_stream The parent tee input stream
 * @return struct aws_input_stream* Pointer to the new branch stream, or NULL on failure
 */
AWS_IO_API struct aws_input_stream *aws_tee_input_stream_new_branch(
    struct aws_input_stream *tee_stream);

/**
 * @brief Checks if the given stream is a tee input stream.
 *
 * @param stream Stream to check
 * @return true If the stream is a tee input stream, false otherwise
 */
AWS_IO_API bool aws_input_stream_is_tee_stream(
    const struct aws_input_stream *stream);

AWS_EXTERN_C_END

#endif /* AWS_IO_TEE_INPUT_STREAM_H */