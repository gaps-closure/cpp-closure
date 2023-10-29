#ifdef _cplusplus
extern "C" {
#endif /* _cplusplus */

#include "codec.h"

void nextrpc_print (nextrpc_datatype *nextrpc) {
  fprintf(stderr, "nextrpc(len=%ld): ", sizeof(*nextrpc));
  fprintf(stderr, " %d,", nextrpc->mux);
  fprintf(stderr, " %d,", nextrpc->sec);
  fprintf(stderr, " %d,", nextrpc->typ);
  fprintf(stderr, " %u, %u, %u, %hu, %hu\n",
          nextrpc->trailer.seq,
          nextrpc->trailer.rqr,
          nextrpc->trailer.oid,
          nextrpc->trailer.mid,
          nextrpc->trailer.crc);
}

void nextrpc_data_encode (void *buff_out, void *buff_in, size_t *len_out) {
  nextrpc_datatype *p1 = (nextrpc_datatype *) buff_in;
  nextrpc_output   *p2 = (nextrpc_output *)   buff_out;
  p2->mux = htonl(p1->mux);
  p2->sec = htonl(p1->sec);
  p2->typ = htonl(p1->typ);
  p2->trailer.seq = htonl(p1->trailer.seq);
  p2->trailer.rqr = htonl(p1->trailer.rqr);
  p2->trailer.oid = htonl(p1->trailer.oid);
  p2->trailer.mid = htons(p1->trailer.mid);
  p2->trailer.crc = htons(p1->trailer.crc);
  *len_out = sizeof(int32_t) + sizeof(int32_t) + sizeof(int32_t) + sizeof(trailer_datatype);
}

void nextrpc_data_decode (void *buff_out, void *buff_in, size_t *len_in) {
  nextrpc_output   *p1 = (nextrpc_output *)   buff_in;
  nextrpc_datatype *p2 = (nextrpc_datatype *) buff_out;
  p2->mux = ntohl(p1->mux);
  p2->sec = ntohl(p1->sec);
  p2->typ = ntohl(p1->typ);
  p2->trailer.seq = ntohl(p1->trailer.seq);
  p2->trailer.rqr = ntohl(p1->trailer.rqr);
  p2->trailer.oid = ntohl(p1->trailer.oid);
  p2->trailer.mid = ntohs(p1->trailer.mid);
  p2->trailer.crc = ntohs(p1->trailer.crc);
}

void okay_print (okay_datatype *okay) {
  fprintf(stderr, "okay(len=%ld): ", sizeof(*okay));
  fprintf(stderr, " %d,", okay->x);
  fprintf(stderr, " %u, %u, %u, %hu, %hu\n",
          okay->trailer.seq,
          okay->trailer.rqr,
          okay->trailer.oid,
          okay->trailer.mid,
          okay->trailer.crc);
}

void okay_data_encode (void *buff_out, void *buff_in, size_t *len_out) {
  okay_datatype *p1 = (okay_datatype *) buff_in;
  okay_output   *p2 = (okay_output *)   buff_out;
  p2->x = htonl(p1->x);
  p2->trailer.seq = htonl(p1->trailer.seq);
  p2->trailer.rqr = htonl(p1->trailer.rqr);
  p2->trailer.oid = htonl(p1->trailer.oid);
  p2->trailer.mid = htons(p1->trailer.mid);
  p2->trailer.crc = htons(p1->trailer.crc);
  *len_out = sizeof(int32_t) + sizeof(trailer_datatype);
}

void okay_data_decode (void *buff_out, void *buff_in, size_t *len_in) {
  okay_output   *p1 = (okay_output *)   buff_in;
  okay_datatype *p2 = (okay_datatype *) buff_out;
  p2->x = ntohl(p1->x);
  p2->trailer.seq = ntohl(p1->trailer.seq);
  p2->trailer.rqr = ntohl(p1->trailer.rqr);
  p2->trailer.oid = ntohl(p1->trailer.oid);
  p2->trailer.mid = ntohs(p1->trailer.mid);
  p2->trailer.crc = ntohs(p1->trailer.crc);
}

void request_extraconstructor_print (request_extraconstructor_datatype *request_extraconstructor) {
  fprintf(stderr, "request_extraconstructor(len=%ld): ", sizeof(*request_extraconstructor));
  fprintf(stderr, " %lu,", request_extraconstructor->oid);
  fprintf(stderr, " %u, %u, %u, %hu, %hu\n",
          request_extraconstructor->trailer.seq,
          request_extraconstructor->trailer.rqr,
          request_extraconstructor->trailer.oid,
          request_extraconstructor->trailer.mid,
          request_extraconstructor->trailer.crc);
}

void request_extraconstructor_data_encode (void *buff_out, void *buff_in, size_t *len_out) {
  request_extraconstructor_datatype *p1 = (request_extraconstructor_datatype *) buff_in;
  request_extraconstructor_output   *p2 = (request_extraconstructor_output *)   buff_out;
  p2->oid = htonll(p1->oid);
  p2->trailer.seq = htonl(p1->trailer.seq);
  p2->trailer.rqr = htonl(p1->trailer.rqr);
  p2->trailer.oid = htonl(p1->trailer.oid);
  p2->trailer.mid = htons(p1->trailer.mid);
  p2->trailer.crc = htons(p1->trailer.crc);
  *len_out = sizeof(uint64_t) + sizeof(trailer_datatype);
}

void request_extraconstructor_data_decode (void *buff_out, void *buff_in, size_t *len_in) {
  request_extraconstructor_output   *p1 = (request_extraconstructor_output *)   buff_in;
  request_extraconstructor_datatype *p2 = (request_extraconstructor_datatype *) buff_out;
  p2->oid = ntohll(p1->oid);
  p2->trailer.seq = ntohl(p1->trailer.seq);
  p2->trailer.rqr = ntohl(p1->trailer.rqr);
  p2->trailer.oid = ntohl(p1->trailer.oid);
  p2->trailer.mid = ntohs(p1->trailer.mid);
  p2->trailer.crc = ntohs(p1->trailer.crc);
}

void request_extragetvalue_print (request_extragetvalue_datatype *request_extragetvalue) {
  fprintf(stderr, "request_extragetvalue(len=%ld): ", sizeof(*request_extragetvalue));
  fprintf(stderr, " %lu,", request_extragetvalue->oid);
  fprintf(stderr, " %u, %u, %u, %hu, %hu\n",
          request_extragetvalue->trailer.seq,
          request_extragetvalue->trailer.rqr,
          request_extragetvalue->trailer.oid,
          request_extragetvalue->trailer.mid,
          request_extragetvalue->trailer.crc);
}

void request_extragetvalue_data_encode (void *buff_out, void *buff_in, size_t *len_out) {
  request_extragetvalue_datatype *p1 = (request_extragetvalue_datatype *) buff_in;
  request_extragetvalue_output   *p2 = (request_extragetvalue_output *)   buff_out;
  p2->oid = htonll(p1->oid);
  p2->trailer.seq = htonl(p1->trailer.seq);
  p2->trailer.rqr = htonl(p1->trailer.rqr);
  p2->trailer.oid = htonl(p1->trailer.oid);
  p2->trailer.mid = htons(p1->trailer.mid);
  p2->trailer.crc = htons(p1->trailer.crc);
  *len_out = sizeof(uint64_t) + sizeof(trailer_datatype);
}

void request_extragetvalue_data_decode (void *buff_out, void *buff_in, size_t *len_in) {
  request_extragetvalue_output   *p1 = (request_extragetvalue_output *)   buff_in;
  request_extragetvalue_datatype *p2 = (request_extragetvalue_datatype *) buff_out;
  p2->oid = ntohll(p1->oid);
  p2->trailer.seq = ntohl(p1->trailer.seq);
  p2->trailer.rqr = ntohl(p1->trailer.rqr);
  p2->trailer.oid = ntohl(p1->trailer.oid);
  p2->trailer.mid = ntohs(p1->trailer.mid);
  p2->trailer.crc = ntohs(p1->trailer.crc);
}

void response_eaxtragetvalueret_print (response_eaxtragetvalueret_datatype *response_eaxtragetvalueret) {
  fprintf(stderr, "response_eaxtragetvalueret(len=%ld): ", sizeof(*response_eaxtragetvalueret));
  fprintf(stderr, " %lu,", response_eaxtragetvalueret->oid);
  fprintf(stderr, " %d,", response_eaxtragetvalueret->value);
  fprintf(stderr, " %u, %u, %u, %hu, %hu\n",
          response_eaxtragetvalueret->trailer.seq,
          response_eaxtragetvalueret->trailer.rqr,
          response_eaxtragetvalueret->trailer.oid,
          response_eaxtragetvalueret->trailer.mid,
          response_eaxtragetvalueret->trailer.crc);
}

void response_eaxtragetvalueret_data_encode (void *buff_out, void *buff_in, size_t *len_out) {
  response_eaxtragetvalueret_datatype *p1 = (response_eaxtragetvalueret_datatype *) buff_in;
  response_eaxtragetvalueret_output   *p2 = (response_eaxtragetvalueret_output *)   buff_out;
  p2->oid = htonll(p1->oid);
  p2->value = htonl(p1->value);
  p2->trailer.seq = htonl(p1->trailer.seq);
  p2->trailer.rqr = htonl(p1->trailer.rqr);
  p2->trailer.oid = htonl(p1->trailer.oid);
  p2->trailer.mid = htons(p1->trailer.mid);
  p2->trailer.crc = htons(p1->trailer.crc);
  *len_out = sizeof(uint64_t) + sizeof(int32_t) + sizeof(trailer_datatype);
}

void response_eaxtragetvalueret_data_decode (void *buff_out, void *buff_in, size_t *len_in) {
  response_eaxtragetvalueret_output   *p1 = (response_eaxtragetvalueret_output *)   buff_in;
  response_eaxtragetvalueret_datatype *p2 = (response_eaxtragetvalueret_datatype *) buff_out;
  p2->oid = ntohll(p1->oid);
  p2->value = ntohl(p1->value);
  p2->trailer.seq = ntohl(p1->trailer.seq);
  p2->trailer.rqr = ntohl(p1->trailer.rqr);
  p2->trailer.oid = ntohl(p1->trailer.oid);
  p2->trailer.mid = ntohs(p1->trailer.mid);
  p2->trailer.crc = ntohs(p1->trailer.crc);
}


#ifdef _cplusplus
}
#endif /* _cplusplus */
