/* uninit.c: Implementation of uninitialized page.
 *
 * All of the pages are born as uninit page. When the first page fault occurs,
 * the handler chain calls uninit_initialize (page->operations.swap_in).
 * The uninit_initialize function transmutes the page into the specific page
 * object (anon, file, page_cache), by initializing the page object,and calls
 * initialization callback that passed from vm_alloc_page_with_initializer
 * function.
 * */

#include "vm/vm.h"
#include "vm/uninit.h"

static bool uninit_initialize (struct page *page, void *kva);
static void uninit_destroy (struct page *page);

/* DO NOT MODIFY this struct */
static const struct page_operations uninit_ops = {
	.swap_in = uninit_initialize,
	.swap_out = NULL,
	.destroy = uninit_destroy,
	.type = VM_UNINIT,
};

/* DO NOT MODIFY this function */
void
uninit_new (struct page *page, void *va, vm_initializer *init,
		enum vm_type type, void *aux,
		bool (*initializer)(struct page *, enum vm_type, void *)) {
	ASSERT (page != NULL);

	*page = (struct page) {
		.operations = &uninit_ops,	// operations->swap_in : uninit_initialize
		.va = va,
		.frame = NULL, /* no frame for now */
		.uninit = (struct uninit_page) {
			.init = init,	// lazy_load_segment가 담긴다.(load_segment부터)
			.type = type,
			.aux = aux,
			.page_initializer = initializer,	// anon_initializer | file_backed_initializer
		}
	};
}

/* Initalize the page on first fault */
/* page fault가 처음 발생했을 때, 페이지를 초기화 */
static bool
uninit_initialize (struct page *page, void *kva) {
	/* 인수로 받은 페이지에 union member인 uninit을 저장 */
	struct uninit_page *uninit = &page->uninit;

	/* Fetch first, page_initialize may overwrite the values */
	/* page_initializer 함수가 값을 덮어쓸 수 있으므로 이전에
	 * 가져온 값들을 먼저 저장해야 함 */
	vm_initializer *init = uninit->init;	//lazy_load_segment 호출
	void *aux = uninit->aux;				//fp를 aux에 저장

	/* TODO: You may need to fix this function. */
	/* uninit->page_initializer는 page를 초기화하고, pa를 va에 매핑 */
	/* kva는 uninit_new에서 va로 받은 커널 가상 주소? */
	return uninit->page_initializer (page, uninit->type, kva) &&
		(init ? init (page, aux) : true);
}

/* Free the resources hold by uninit_page. Although most of pages are transmuted
 * to other page objects, it is possible to have uninit pages when the process
 * exit, which are never referenced during the execution.
 * PAGE will be freed by the caller. */
static void
uninit_destroy (struct page *page) {
	struct uninit_page *uninit UNUSED = &page->uninit;
	/* TODO: Fill this function.
	 * TODO: If you don't have anything to do, just return. */
}
