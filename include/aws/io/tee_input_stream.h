/**
 * Copyright Christopher Ochsenreither. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

 #ifndef AWS_IO_TEE_INPUT_STREAM_H
 #define AWS_IO_TEE_INPUT_STREAM_H
 
 #include <aws/io/stream.h> /* Use the correct header from aws-c-io */
 
 AWS_EXTERN_C_BEGIN
 
 /**
  * @brief Creates a new tee input stream that wraps a source stream, allowing the source
  * data to be read multiple times via branches.
  *
  * The tee stream reads from the `source` stream on demand (when a branch needs data
  * not yet buffered) and stores the data in an internal buffer. This allows multiple
  * "branch" streams to be created, each providing an independent view of the source
  * stream's data from the beginning.
  *
  * This is particularly useful for operations like SigV4 signing where the request body
  * needs to be hashed (read once) and also sent in the HTTP request (read again).
  *
  * @warning The current implementation buffers the *entire* source stream content in memory
  * as it's read. This can lead to high memory consumption for large source streams.
  * Consider alternatives if memory usage is a critical concern for large bodies.
  *
  * @warning The tee stream and its branches are not thread-safe. Access must be
  * synchronized externally if used across multiple threads.
  *
  * @param allocator The allocator to use for the tee stream and its internal buffer.
  * @param source The source input stream to wrap. The tee stream takes ownership of
  *               this stream and will destroy it when the tee stream itself is destroyed.
  *               The source stream does not need to be seekable, but if it is not,
  *               seeking on the tee stream or its branches will fail.
  * @return A pointer to the newly created tee input stream (which acts as the primary
  *         branch), or NULL on allocation failure or invalid arguments. The returned
  *         stream should be destroyed using aws_input_stream_destroy().
  */
 AWS_IO_API struct aws_input_stream *aws_tee_input_stream_new(
     struct aws_allocator *allocator,
     struct aws_input_stream *source);
 
 /**
  * @brief Creates a new branch from an existing tee input stream.
  *
  * A branch provides an independent read cursor into the data buffered by the parent
  * tee stream. Reading from one branch does not affect the read position of other branches
  * or the original tee stream. All branches share the same underlying buffered data.
  *
  * Branches can be created at any time. If the source stream has already been partially
  * or fully read by the parent tee stream or other branches, the new branch will still
  * have access to all the data buffered so far, starting from the beginning.
  *
  * @param tee_stream A pointer to an existing tee input stream created by
  *                   `aws_tee_input_stream_new()`. Must not be NULL.
  * @return A pointer to the newly created branch input stream, or NULL on allocation
  *         failure or if `tee_stream` is not a valid tee stream. The returned branch
  *         stream should be destroyed using aws_input_stream_destroy(). Destroying a
  *         branch does not affect the parent tee stream or other branches.
  */
 AWS_IO_API struct aws_input_stream *aws_tee_input_stream_new_branch(
     struct aws_input_stream *tee_stream);
 
 /**
  * @brief Checks if the provided input stream is a tee stream created by
  * `aws_tee_input_stream_new()`.
  *
  * This can be used to determine if it's safe to call `aws_tee_input_stream_new_branch()`
  * on a given stream.
  *
  * @param stream A pointer to the input stream to check. Can be NULL.
  * @return `true` if the stream is a non-NULL, valid tee input stream, `false` otherwise.
  */
 AWS_IO_API bool aws_input_stream_is_tee_stream(
     const struct aws_input_stream *stream);
 
 AWS_EXTERN_C_END
 
 #endif /* AWS_IO_TEE_INPUT_STREAM_H */