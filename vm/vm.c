/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"

/*---Project 3---*/
struct list frame_table;

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

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */
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

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
/* 초기화를 포함한 대기 중인 페이지 객체를 생성합니다. 페이지를 생성하려면 
 * 직접 생성하지 말고 이 함수 vm_alloc_page를 통해 만드세요. */
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) {

	ASSERT (VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current ()->spt;

	/* Check wheter the upage is already occupied or not. */
	if (spt_find_page (spt, upage) == NULL) {
		/* TODO: Create the page, fetch the initialier according to the VM type,
		 * TODO: and then create "uninit" page struct by calling uninit_new. You
		 * TODO: should modify the field after calling the uninit_new. */
		// 1) page를 생성하고,
		struct page *p = (struct page*)malloc(sizeof(struct page));

		// 2) type에 따라 초기화 함수를 가져와서
		bool(*page_initializer)(struct page*, enum vm_type, void*);

		switch(VM_TYPE(type)) {	//최하위 bit 3개 추출
			case VM_ANON:	//001
			page_initializer = anon_initializer;
			break;
			case VM_FILE:	//010
			page_initializer = file_backed_initializer;
			break;
		}
		// 3) "uninit" 타입의 페이지로 초기화한다.
		uninit_new(p, upage, init, type, aux, page_initializer);
		// 필드 수정은 uninit_new를 호출한 이후에 해야한다.
		p->writable = writable;
		
		/* TODO: Insert the page into the spt. */
		// 4) 생성한 페이지를 SPT에 추가한다.
		return spt_insert_page(spt, p);
	}
err:
	return false;
}

/* Find VA from spt and return page. On error, return NULL. */
/* spt 및 return 페이지에서 VA를 찾습니다. 오류가 발생하면 NULL을 반환합니다. */
struct page *
spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) {
	/* TODO: Fill this function. */
	struct page *page = (struct page *)malloc(sizeof(struct page));
	struct hash_elem *e;

	/* 인자로 받은 spt 내에서 va를 키로 전달해서 이를 갖는 page를 리턴한다.)
	hash_find(): hash_elem을 리턴해준다. 이로부터 우리는 해당 page를 찾을 수 있어.
	해당 spt의 hash 테이블 구조체를 인자로 넣자. 해당 해시 테이블에서 찾아야 하니까.
	근데 우리가 받은 건 va 뿐이다. 근데 hash_find()는 hash_elem을 인자로 받아야 하니
	dummy page 하나를 만들고 그것의 가상주소를 va로 만들어. 그 다음 이 페이지의 hash
	_elem을 넣는다.
	*/
	// va에 해당하는 hash_elem 찾기
    page->va = va;
    e = hash_find(&spt, &page->hash_elem);

    // 있으면 e에 해당하는 페이지 반환
    return e != NULL ? hash_entry(e, struct page, hash_elem) : NULL;
}

/* Insert PAGE into spt with validation. */
bool
spt_insert_page (struct supplemental_page_table *spt UNUSED,
		struct page *page UNUSED) {
	int succ = false;
	/* TODO: Fill this function. */

	return hash_insert(&spt, &page->hash_elem) == NULL ? true : false;
	}
void
spt_remove_page (struct supplemental_page_table *spt, struct page *page) {
	vm_dealloc_page (page);
	return true;
}

/* Get the struct frame, that will be evicted. */
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
	void *kva = palloc_get_page(PAL_USER); // user pool에서 새로운 physical page를 가져온다.

	if (kva == NULL)   // page 할당 실패 -> 나중에 swap_out 처리
    	PANIC("todo"); // OS를 중지시키고, 소스 파일명, 라인 번호, 함수명 등의 정보와 함께 사용자 지정 메시지를 출력

    frame = malloc(sizeof(struct frame));	// 프레임 할당
    frame->kva = kva;				// 프레임 멤버 초기화
	
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
	// spt에서 va에 해당하는 page 찾기
	page = spt_find_page(&thread_current()->spt,va);
	if(page==NULL)
		return false;
	return vm_do_claim_page (page);
}

/* Claim the PAGE and set up the mmu. */
/* 페이지를 요구하고 MMU를 설정합니다. */
static bool
vm_do_claim_page (struct page *page) {
	struct frame *frame = vm_get_frame ();

	/* Set links */
	frame->page = page;
	page->frame = frame;

	/* TODO: Insert page table entry to map page's VA to frame's PA. */
    // 가상 주소와 물리 주소를 매핑
    struct thread *current = thread_current();
    pml4_set_page(current->pml4, page->va, frame->kva, page->writable);
	/* 첫번째 page_fault에서 이어지는 vm_do_claim_page인 경우
	 * 페이지가 실제로 로딩될 때 호출되는 swap_in은 uninit */
	return swap_in (page, frame->kva); // uninit_initialize
}

/* Returns a hash value for page p. */
unsigned page_hash(const struct hash_elem *p_, void *aux UNUSED)
{
    const struct page *p = hash_entry(p_, struct page, hash_elem);
    return hash_bytes(&p->va, sizeof p->va);
}

/* Returns true if page a precedes page b. */
bool page_less(const struct hash_elem *a_,
               const struct hash_elem *b_, void *aux UNUSED)
{
    const struct page *a = hash_entry(a_, struct page, hash_elem);
    const struct page *b = hash_entry(b_, struct page, hash_elem);

    return a->va < b->va;
}

/* page의 member hash_elem을 인자로 받은 hash_table에 삽입 */
bool page_insert(struct hash *h, struct page *p) {
	
    if(!hash_insert(h, &p->hash_elem))
		return true;
	else
		return false;

}

/* delete in page's member that hash_elem */
bool page_delete(struct hash *h, struct page *p) {
	if(!hash_delete(h, &p->hash_elem)) {
		return true;
	}
	else
		return false;
}

/* Initialize new supplemental page table */
void
supplemental_page_table_init (struct supplemental_page_table *spt UNUSED) {
	/* --- Project 3: VM --- */
	hash_init(&spt->spt_hash, page_hash, page_less, NULL);
}

/*---Project 3: VM ---*/
unsigned page_hash(const struct hash_elem *p_, void *aux UNUSED) {
	const struct page *p = hash_entry(p_, struct page, hash_elem);
	return hash_bytes(&p->va, sizeof(p->va));
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
