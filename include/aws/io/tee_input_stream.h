/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#ifndef AWS_IO_TEE_INPUT_STREAM_H
#define AWS_IO_TEE_INPUT_STREAM_H

#include <aws/io/input_stream.h>

AWS_EXTERN_C_BEGIN

/**
 * Creates a new tee input stream that allows the same data to be read multiple times.
 * The tee stream reads from the source stream once and buffers the data internally.
 * This allows the data to be read multiple times without consuming the source stream.
 * 
 * This is particularly useful for operations like SigV4 signing where the body needs
 * to be hashed but also sent in the request.
 *
 * @param allocator Memory allocator to use.
 * @param source Source stream to read from. The tee stream takes ownership of this stream.
 * @return A new tee input stream, or NULL on failure.
 */
AWS_IO_API struct aws_input_stream *aws_tee_input_stream_new(
    struct aws_allocator *allocator,
    struct aws_input_stream *source);

/**
 * Creates a new branch from a tee input stream.
 * Each branch can be read independently, but all branches share the same underlying data.
 * 
 * @param tee_stream The tee stream to create a branch from.
 * @return A new input stream branch, or NULL on failure.
 */
AWS_IO_API struct aws_input_stream *aws_tee_input_stream_new_branch(
    struct aws_input_stream *tee_stream);

/**
 * Checks if a stream is a tee input stream.
 * 
 * @param stream The stream to check.
 * @return true if the stream is a tee input stream, false otherwise.
 */
AWS_IO_API bool aws_input_stream_is_tee_stream(
    const struct aws_input_stream *stream);

AWS_EXTERN_C_END

#endif /* AWS_IO_TEE_INPUT_STREAM_H */
