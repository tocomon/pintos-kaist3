#ifndef __LIB_KERNEL_HASH_H
#define __LIB_KERNEL_HASH_H

/* Hash table.
 *
 * This data structure is thoroughly documented in the Tour of
 * Pintos for Project 3.
 *
 * This is a standard hash table with chaining.  To locate an
 * element in the table, we compute a hash function over the
 * element's data and use that as an index into an array of
 * doubly linked lists, then linearly search the list.
 *
 * The chain lists do not use dynamic allocation.  Instead, each
 * structure that can potentially be in a hash must embed a
 * struct hash_elem member.  All of the hash functions operate on
 * these `struct hash_elem's.  The hash_entry macro allows
 * conversion from a struct hash_elem back to a structure object
 * that contains it.  This is the same technique used in the
 * linked list implementation.  Refer to lib/kernel/list.h for a
 * detailed explanation. */
/* 해시 테이블.
 *
 * 이 데이터 구조는 Pintos의 프로젝트 3을 위해 철저히 문서화되어 
 * 있습니다.
 *
 * 이는 체이닝 방식을 사용한 표준 해시 테이블입니다. 테이블에서 요
 * 소를 찾기 위해, 요소의 데이터를 해시 함수를 통해 해시 값을 계산
 * 하고, 이를 이중 연결 리스트 배열의 인덱스로 사용한 다음 리스트
 * 에서 선형 검색을 수행합니다.
 *
 * 체인 리스트는 동적 할당을 사용하지 않습니다. 대신, 해시에 속할 수
 * 있는 각 구조체는 struct hash_elem 멤버를 포함해야 합니다. 모든 해
 * 시 함수는 이 `struct hash_elem`에 대해 작동합니다. hash_entry 매크
 * 로를 사용하면 struct hash_elem을 포함하는 struct page로의 변환도 
 * 가능합니다. 이는 연결 리스트 구현에서 사용하는 기술과 동일합니다.
 * 자세한 설명은 lib/kernel/list.h를 참조하십시오. */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "list.h"

/* Hash element. */
struct hash_elem {
	struct list_elem list_elem;
};

/* Converts pointer to hash element HASH_ELEM into a pointer to
 * the structure that HASH_ELEM is embedded inside.  Supply the
 * name of the outer structure STRUCT and the member name MEMBER
 * of the hash element.  See the big comment at the top of the
 * file for an example. */
/* 해시 엘리먼트 HASH_ELEM에 대한 포인터를 HASH_ELEM이 포함된 구
 * 조에 대한 포인터로 변환합니다. 외부 구조체의 이름 STRUCT와 
 * 해시 엘리먼트의 멤버 이름 MEMBER를 입력합니다. 예제는 파일 
 * 상단의 큰 주석을 참조하세요. */

/* 예제는 즉, hash_elem을 가리키는 주소(포인터)를 hash_elem을 포
 * 함하는 struct page를 가리키는 주소로 변환해준다. 방법은 
 * hash_elem의 member인 list elem을 참조한 주소 - 구조체멤버의 오프셋
 * 
 * 반환값: hash_elem이 들어있는 page 주소
 * 
 * offsetof는 gcc 컴파일러가 제공하는 내장함수를 이용한 매크로
 * : 구조체의 멤버의 offset을 계산하는데 사용된다. 구조체의 시작
 * 위치로부터 얼마나 떨어져 있는지 계산하여 반환한다.
 * type, member를 인수로 받는다(여기선 멤버 참조를 한번 더 하여 
 * 오프셋의 오프셋을 계산한 값이 반환된다.) */

#define hash_entry(HASH_ELEM, STRUCT, MEMBER)                   \
	((STRUCT *) ((uint8_t *) &(HASH_ELEM)->list_elem        \
		- offsetof (STRUCT, MEMBER.list_elem)))

/* Computes and returns the hash value for hash element E, given
 * auxiliary data AUX. */
/* 보조 데이터 AUX를 사용하여 해시 요소 E의 해시 값을 계산하고 반
 * 환합니다. */  
typedef uint64_t hash_hash_func (const struct hash_elem *e, void *aux);

/* Compares the value of two hash elements A and B, given
 * auxiliary data AUX.  Returns true if A is less than B, or
 * false if A is greater than or equal to B. */
/* 보조 데이터 AUX를 사용하여 해시 요소 A와 B의 값을 비교합니다.
 * A가 B보다 작으면 true를 반환하고, A가 B보다 크거나 같으면 fa
 * lse를 반환합니다. */
typedef bool hash_less_func (const struct hash_elem *a,
		const struct hash_elem *b,
		void *aux);

/* Performs some operation on hash element E, given auxiliary
 * data AUX. */
typedef void hash_action_func (struct hash_elem *e, void *aux);

/* Hash table. */
struct hash {
	size_t elem_cnt;            /* Number of elements in table. */
	size_t bucket_cnt;          /* Number of buckets, a power of 2. */
	struct list *buckets;       /* Array of `bucket_cnt' lists. */
	hash_hash_func *hash;       /* Hash function. */
	hash_less_func *less;       /* Comparison function. */
	void *aux;                  /* Auxiliary data for `hash' and `less'. */
};

/* A hash table iterator. */
struct hash_iterator {
	struct hash *hash;          /* The hash table. */
	struct list *bucket;        /* Current bucket. */
	struct hash_elem *elem;     /* Current hash element in current bucket. */
};

/* Basic life cycle. */
bool hash_init (struct hash *, hash_hash_func *, hash_less_func *, void *aux);
void hash_clear (struct hash *, hash_action_func *);
void hash_destroy (struct hash *, hash_action_func *);

/* Search, insertion, deletion. */
struct hash_elem *hash_insert (struct hash *, struct hash_elem *);
struct hash_elem *hash_replace (struct hash *, struct hash_elem *);
struct hash_elem *hash_find (struct hash *, struct hash_elem *);
struct hash_elem *hash_delete (struct hash *, struct hash_elem *);

/* Iteration. */
void hash_apply (struct hash *, hash_action_func *);
void hash_first (struct hash_iterator *, struct hash *);
struct hash_elem *hash_next (struct hash_iterator *);
struct hash_elem *hash_cur (struct hash_iterator *);

/* Information. */
size_t hash_size (struct hash *);
bool hash_empty (struct hash *);

/* Sample hash functions. */
uint64_t hash_bytes (const void *, size_t);
uint64_t hash_string (const char *);
uint64_t hash_int (int);

#endif /* lib/kernel/hash.h */
