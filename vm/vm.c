/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"
#include "threads/thread.h"
#include "vm/inspect.h"
#include "userprog/process.h"
/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void
vm_init (void) {
	vm_anon_init ();
	vm_file_init ();
#ifdef EFILESYS  /* For project 4 */
	pagecache_init ();
#endif
	register_inspect_intr ();
	/* DO NOT MODIFY UPPER LINES. */
	/* TODO: Your code goes here. */
}

/* 이 함수는 페이지의 유형을 가져오는 함수다. 이 함수는 페이지가 초기화된 후의 유형을 알고자 할 때 유용하다. */
enum vm_type
page_get_type (struct page *page) {
	int ty = VM_TYPE (page->operations->type);
	switch (ty) {
		case VM_UNINIT:
			return VM_TYPE (page->uninit.type);
		default:
			return ty;
	}
}

/* Helpers */
static struct frame *vm_get_victim (void);
static bool vm_do_claim_page (struct page *page);
static struct frame *vm_evict_frame (void);

/* pending page객체를 초기화하는 함수인 vm_create_pending_page를 사용하여 페이지를 생성하라
	페이지를 생성하려면 직접 생성하지말고 이 함수 혹은 vm_alloc_page를 통해 생성하라 */
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) {

	ASSERT (VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current ()->spt;

	/* Check wheter the upage is already occupied or not. */
	if (spt_find_page (spt, upage) == NULL) {
		/* 페이지를 생성하고 VM유형에 따라 초기화 함수를 가져와서 uninit페이지 구조체를 호출하여 생성
		 uninit _new 를 호출한 후에 필드를 수정해야한다.*/

		/* TODO: 페이지를 spt에 삽입하라 */
	}
err:
	return false;
}

/* spt에서 va를 찾아서 해당 페이지를 반환하는 역할
   코드를 수정하ㅕ spt와 va에 따라 페이지를 찾고 해당 페이지를 반환하도록 구현
   아마 spt(보조 페이지 테이블)에서 주어진 가상 주소에 해당하는 페이지를 찾는 역할. */
struct page *
spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) {
	struct page *page = NULL;
	/* TODO: Fill this function. */

	return page;
}

/* Insert PAGE into spt with validation. */
bool
spt_insert_page (struct supplemental_page_table *spt UNUSED,
		struct page *page UNUSED) {
	int succ = false;
	/* TODO: Fill this function. */

	return succ;
}

void
spt_remove_page (struct supplemental_page_table *spt, struct page *page) {
	vm_dealloc_page (page);
	return true;
}

/* 퇴출될 프레임을 가져오는 역할, 어떤 프레임을 퇴출할지 선택하는 기준이 필요하며, 해당 기준에 따라 퇴출될프레임을 가리키는 포인터
	victim을 반환해야한다.*/
static struct frame *
vm_get_victim (void) {
	struct frame *victim = NULL;
	 /* TODO: The policy for eviction is up to you. */

	return victim;
}

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
static struct frame *
vm_evict_frame (void) {
	struct frame *victim UNUSED = vm_get_victim ();
	/* TODO: swap out the victim and return the evicted frame. */

	return NULL;
}

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
static struct frame *
vm_get_frame (void) {
	struct frame *frame = NULL;
	/* TODO: Fill this function. */

	ASSERT (frame != NULL);
	ASSERT (frame->page == NULL);
	return frame;
}

/* Growing the stack. */
static void
vm_stack_growth (void *addr UNUSED) {
}

/* Handle the fault on write_protected page */
static bool
vm_handle_wp (struct page *page UNUSED) {
}

/* Return true on success */
bool
vm_try_handle_fault (struct intr_frame *f UNUSED, void *addr UNUSED,
		bool user UNUSED, bool write UNUSED, bool not_present UNUSED) {
	struct supplemental_page_table *spt UNUSED = &thread_current ()->spt;
	struct page *page = NULL;
	/* TODO: Validate the fault */
	/* TODO: Your code goes here */

	return vm_do_claim_page (page);
}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
void
vm_dealloc_page (struct page *page) {
	destroy (page);
	free (page);
}

/* Claim the page that allocate on VA. */
bool
vm_claim_page (void *va UNUSED) {
	struct page *page = NULL;
	/* TODO: Fill this function */

	return vm_do_claim_page (page);
}

/* Claim the PAGE and set up the mmu. */
static bool
vm_do_claim_page (struct page *page) {
	struct frame *frame = vm_get_frame ();

	/* Set links */
	frame->page = page;
	page->frame = frame;

	/* TODO: Insert page table entry to map page's VA to frame's PA. */

	return swap_in (page, frame->kva);
}
/* 이 함수를 왜 썼지 ?? */

unsigned page_hash_func (const struct hash_elem *e , void *aux){
	const struct page *p = hash_entry(e,struct page, hash_elem);
	return hash_bytes(&p->va, sizeof(p->va));
}
/* 해시테이블은 키의 순서를 기준으로 정렬된 순서를 유지해야한다. 두개의 해시 엘리멘트를 비교
va는 가상메모리 주소*/
static unsigned page_less(const struct hash_elem *a,const struct hash_elem *b, void *aux){
	const struct page *p_1 = hash_entry(a,struct page, hash_elem);
	const struct page *p_2 = hash_entry(b, struct page, hash_elem);
	return p_1 -> va < p_2 -> va;
}

bool insert_page(struct hash *h, struct page *p){
	if(!hash_insert(h, &p->hash_elem)){
		return true;
	}
	else{
		return false;
	}
}
bool delete_page(struct hash *h, struct page *p){
	if(!hash_delete(h, &p->hash_elem)){
		return true;
	}
	else {
		return false;
	}
}

/* Initialize new supplemental page table */
void supplemental_page_table_init (struct supplemental_page_table *spt UNUSED) {
	hash_init(&spt->spt_hash,insert_page, delete_page,NULL);
}
struct page *spt_find_page (struct supplemental_page_table *spt,void *va) {
	struct page *page = (struct page*)malloc(sizeof(struct page));
	struct hash_elem *e;
	/* 구조체 page인스턴스 동적 할당 +hash_elem인스턴스 데이터 구조에서 각 요소에 대한 메모리를 할당하고 해당 요소를 해시테이블에 
	삽입하기 위해 사용*/
	/*hash_find()은 hash_elem을 리턴해준다 이로부터 해당page를 찾을 수 있다.*/
	// 해당 va가 속해있는 page를 만든다
	page->va = pg_round_down(va);
	// e와 같은 해시값을 갖는 page를 spt에서 찾은 다음 해당 hash_elem을 리턴
	e = hash_find(&spt->spt_hash, &page->hash_elem);

	free(page);
	return e!=NULL ? hash_entry(e,struct page, hash_elem) : NULL;

}

bool spt_insert_page(struct supplemental_page_table *spt, struct page *page){
	int succ = false;
	if(!hash_insert(&spt->spt_hash,&page->hash_elem))  // hash_insert는 삽입 동작의 성공여부를 확인한다.
	//동일 해시값이 존재할 경우 페이지 삽입이 실패하고 새로운 페이지가 중복되는 것으로 간주 
		succ = true;

	return succ;
}
/* Copy supplemental page table from src to dst */
bool
supplemental_page_table_copy (struct supplemental_page_table *dst UNUSED,
		struct supplemental_page_table *src UNUSED) {
}

/* Free the resource hold by the supplemental page table */
void
supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) {
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
}
