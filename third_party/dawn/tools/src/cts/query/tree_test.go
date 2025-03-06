package query_test

import (
	"fmt"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"github.com/google/go-cmp/cmp"
)

var (
	abort   = "Abort"
	crash   = "Crash"
	failure = "Failure"
	pass    = "Pass"
	skip    = "Skip"
)

func NewTree[Data any](t *testing.T, entries ...query.QueryData[Data]) (query.Tree[Data], error) {
	return query.NewTree(entries...)
}

func TestNewSingle(t *testing.T) {
	type Tree = query.Tree[string]
	type Node = query.TreeNode[string]
	type QueryData = query.QueryData[string]
	type Children = query.TreeNodeChildren[string]

	type Test struct {
		in     QueryData
		expect Tree
	}
	for _, test := range []Test{
		{ /////////////////////////////////////////////////////////////////////
			in: QueryData{
				Query: Q(`suite:*`),
				Data:  pass,
			},
			expect: Tree{
				TreeNode: Node{
					Children: Children{
						query.TreeNodeChildKey{`suite`, query.Suite}: {
							Query: Q(`suite`),
							Children: Children{
								query.TreeNodeChildKey{`*`, query.Files}: {
									Query: Q(`suite:*`),
									Data:  &pass,
								},
							},
						},
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////
			in: QueryData{
				Query: Q(`suite:a,*`),
				Data:  pass,
			},
			expect: Tree{
				TreeNode: Node{
					Children: Children{
						query.TreeNodeChildKey{`suite`, query.Suite}: {
							Query: Q(`suite`),
							Children: Children{
								query.TreeNodeChildKey{`a`, query.Files}: {
									Query: Q(`suite:a`),
									Children: Children{
										query.TreeNodeChildKey{`*`, query.Files}: {
											Query: Q(`suite:a,*`),
											Data:  &pass,
										},
									},
								},
							},
						},
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////
			in: QueryData{
				Query: Q(`suite:a,b:*`),
				Data:  pass,
			},
			expect: Tree{
				TreeNode: Node{
					Children: Children{
						query.TreeNodeChildKey{`suite`, query.Suite}: {
							Query: Q(`suite`),
							Children: Children{
								query.TreeNodeChildKey{`a`, query.Files}: {
									Query: Q(`suite:a`),
									Children: Children{
										query.TreeNodeChildKey{`b`, query.Files}: {
											Query: Q(`suite:a,b`),
											Children: Children{
												query.TreeNodeChildKey{`*`, query.Tests}: {
													Query: Q(`suite:a,b:*`),
													Data:  &pass,
												},
											},
										},
									},
								},
							},
						},
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////
			in: QueryData{
				Query: Q(`suite:a,b:c:*`),
				Data:  pass,
			},
			expect: Tree{
				TreeNode: Node{
					Children: Children{
						query.TreeNodeChildKey{`suite`, query.Suite}: {
							Query: Q(`suite`),
							Children: Children{
								query.TreeNodeChildKey{`a`, query.Files}: {
									Query: Q(`suite:a`),
									Children: Children{
										query.TreeNodeChildKey{`b`, query.Files}: {
											Query: Q(`suite:a,b`),
											Children: Children{
												query.TreeNodeChildKey{`c`, query.Tests}: {
													Query: Q(`suite:a,b:c`),
													Children: Children{
														query.TreeNodeChildKey{`*`, query.Cases}: {
															Query: Q(`suite:a,b:c:*`),
															Data:  &pass,
														},
													},
												},
											},
										},
									},
								},
							},
						},
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////
			in: QueryData{
				Query: Q(`suite:a,b,c:d,e:f="g";h=[1,2,3];i=4;*`),
				Data:  pass,
			},
			expect: Tree{
				TreeNode: Node{
					Children: Children{
						query.TreeNodeChildKey{`suite`, query.Suite}: {
							Query: Q(`suite`),
							Children: Children{
								query.TreeNodeChildKey{`a`, query.Files}: {
									Query: Q(`suite:a`),
									Children: Children{
										query.TreeNodeChildKey{`b`, query.Files}: {
											Query: Q(`suite:a,b`),
											Children: Children{
												query.TreeNodeChildKey{`c`, query.Files}: {
													Query: Q(`suite:a,b,c`),
													Children: Children{
														query.TreeNodeChildKey{`d`, query.Tests}: {
															Query: Q(`suite:a,b,c:d`),
															Children: Children{
																query.TreeNodeChildKey{`e`, query.Tests}: {
																	Query: Q(`suite:a,b,c:d,e`),
																	Children: Children{
																		query.TreeNodeChildKey{`f="g";h=[1,2,3];i=4;*`, query.Cases}: {
																			Query: Q(`suite:a,b,c:d,e:f="g";h=[1,2,3];i=4;*`),
																			Data:  &pass,
																		},
																	},
																},
															},
														},
													},
												},
											},
										},
									},
								},
							},
						},
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////
			in: QueryData{
				Query: Q(`suite:a,b:c:d="e";*`), Data: pass,
			},
			expect: Tree{
				TreeNode: Node{
					Children: Children{
						query.TreeNodeChildKey{`suite`, query.Suite}: {
							Query: Q(`suite`),
							Children: Children{
								query.TreeNodeChildKey{`a`, query.Files}: {
									Query: Q(`suite:a`),
									Children: Children{
										query.TreeNodeChildKey{`b`, query.Files}: {
											Query: Q(`suite:a,b`),
											Children: Children{
												query.TreeNodeChildKey{`c`, query.Tests}: {
													Query: Q(`suite:a,b:c`),
													Children: Children{
														query.TreeNodeChildKey{`d="e";*`, query.Cases}: {
															Query: Q(`suite:a,b:c:d="e";*`),
															Data:  &pass,
														},
													},
												},
											},
										},
									},
								},
							},
						},
					},
				},
			},
		},
	} {
		got, err := NewTree(t, test.in)
		if err != nil {
			t.Errorf("NewTree(%v): %v", test.in, err)
			continue
		}
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("NewTree(%v) tree was not as expected:\n%v", test.in, diff)
		}
	}
}

func TestNewMultiple(t *testing.T) {
	type Tree = query.Tree[string]
	type Node = query.TreeNode[string]
	type QueryData = query.QueryData[string]
	type Children = query.TreeNodeChildren[string]

	got, err := NewTree(t,
		QueryData{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
		QueryData{Query: Q(`suite:h,b:c:f="g";*`), Data: abort},
		QueryData{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
	)
	if err != nil {
		t.Fatalf("NewTree() returned %v", err)
	}

	expect := Tree{
		TreeNode: Node{
			Children: Children{
				query.TreeNodeChildKey{`suite`, query.Suite}: {
					Query: Q(`suite`),
					Children: Children{
						query.TreeNodeChildKey{`a`, query.Files}: {
							Query: Q(`suite:a`),
							Children: Children{
								query.TreeNodeChildKey{`b`, query.Files}: {
									Query: Q(`suite:a,b`),
									Children: Children{
										query.TreeNodeChildKey{`c`, query.Tests}: {
											Query: Q(`suite:a,b:c`),
											Children: Children{
												query.TreeNodeChildKey{`d="e";*`, query.Cases}: {
													Query: Q(`suite:a,b:c:d="e";*`),
													Data:  &failure,
												},
												query.TreeNodeChildKey{`f="g";*`, query.Cases}: {
													Query: Q(`suite:a,b:c:f="g";*`),
													Data:  &skip,
												},
											},
										},
									},
								},
							},
						},
						query.TreeNodeChildKey{`h`, query.Files}: {
							Query: query.Query{
								Suite: `suite`,
								Files: `h`,
							},
							Children: Children{
								query.TreeNodeChildKey{`b`, query.Files}: {
									Query: query.Query{
										Suite: `suite`,
										Files: `h,b`,
									},
									Children: Children{
										query.TreeNodeChildKey{`c`, query.Tests}: {
											Query: query.Query{
												Suite: `suite`,
												Files: `h,b`,
												Tests: `c`,
											},
											Children: Children{
												query.TreeNodeChildKey{`f="g";*`, query.Cases}: {
													Query: query.Query{
														Suite: `suite`,
														Files: `h,b`,
														Tests: `c`,
														Cases: `f="g";*`,
													},
													Data: &abort,
												},
											},
										},
									},
								},
							},
						},
					},
				},
			},
		},
	}
	if diff := cmp.Diff(got, expect); diff != "" {
		t.Errorf("NewTree() was not as expected:\n%v", diff)
		t.Errorf("got:\n%v", got)
		t.Errorf("expect:\n%v", expect)
	}
}

func TestNewWithCollision(t *testing.T) {
	type Tree = query.Tree[string]
	type QueryData = query.QueryData[string]

	got, err := NewTree(t,
		QueryData{Query: Q(`suite:a,b:c:*`), Data: failure},
		QueryData{Query: Q(`suite:a,b:c:*`), Data: skip},
	)
	expect := Tree{}
	expectErr := query.ErrDuplicateData{
		Query: Q(`suite:a,b:c:*`),
	}
	if diff := cmp.Diff(err, expectErr); diff != "" {
		t.Errorf("NewTree() error was not as expected:\n%v", diff)
	}
	if diff := cmp.Diff(got, expect); diff != "" {
		t.Errorf("NewTree() was not as expected:\n%v", diff)
	}
}

func TestGlob(t *testing.T) {
	type QueryData = query.QueryData[string]

	tree, err := NewTree(t,
		QueryData{Query: Q(`suite:*`), Data: skip},
		QueryData{Query: Q(`suite:a,*`), Data: failure},
		QueryData{Query: Q(`suite:a,b,*`), Data: failure},
		QueryData{Query: Q(`suite:a,b:c:d;*`), Data: failure},
		QueryData{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
		QueryData{Query: Q(`suite:h,b:c:f="g";*`), Data: abort},
		QueryData{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
		QueryData{Query: Q(`suite:a,b:d:*`), Data: failure},
	)
	if err != nil {
		t.Fatalf("NewTree() returned %v", err)
	}

	type Test struct {
		query     query.Query
		expect    []QueryData
		expectErr error
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite`),
			expect: []QueryData{
				{Query: Q(`suite:*`), Data: skip},
				{Query: Q(`suite:a,*`), Data: failure},
				{Query: Q(`suite:a,b,*`), Data: failure},
				{Query: Q(`suite:a,b:c:d;*`), Data: failure},
				{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
				{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
				{Query: Q(`suite:a,b:d:*`), Data: failure},
				{Query: Q(`suite:h,b:c:f="g";*`), Data: abort},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite:*`),
			expect: []QueryData{
				{Query: Q(`suite:*`), Data: skip},
				{Query: Q(`suite:a,*`), Data: failure},
				{Query: Q(`suite:a,b,*`), Data: failure},
				{Query: Q(`suite:a,b:c:d;*`), Data: failure},
				{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
				{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
				{Query: Q(`suite:a,b:d:*`), Data: failure},
				{Query: Q(`suite:h,b:c:f="g";*`), Data: abort},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite:a`),
			expect: []QueryData{
				{Query: Q(`suite:a,*`), Data: failure},
				{Query: Q(`suite:a,b,*`), Data: failure},
				{Query: Q(`suite:a,b:c:d;*`), Data: failure},
				{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
				{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
				{Query: Q(`suite:a,b:d:*`), Data: failure},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite:a,*`),
			expect: []QueryData{
				{Query: Q(`suite:a,*`), Data: failure},
				{Query: Q(`suite:a,b,*`), Data: failure},
				{Query: Q(`suite:a,b:c:d;*`), Data: failure},
				{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
				{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
				{Query: Q(`suite:a,b:d:*`), Data: failure},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite:a,b`),
			expect: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: failure},
				{Query: Q(`suite:a,b:c:d;*`), Data: failure},
				{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
				{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
				{Query: Q(`suite:a,b:d:*`), Data: failure},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite:a,b,*`),
			expect: []QueryData{
				{Query: Q(`suite:a,b,*`), Data: failure},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite:a,b:c:*`),
			expect: []QueryData{
				{Query: Q(`suite:a,b:c:d;*`), Data: failure},
				{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
				{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite:a,b:c`),
			expect: []QueryData{
				{Query: Q(`suite:a,b:c:d;*`), Data: failure},
				{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
				{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite:a,b:c:d="e";*`),
			expect: []QueryData{
				{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
				{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite:a,b:c:d;*`),
			expect: []QueryData{
				{Query: Q(`suite:a,b:c:d;*`), Data: failure},
				{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query: Q(`suite:a,b:c:f="g";*`),
			expect: []QueryData{
				{Query: Q(`suite:a,b:c:d;*`), Data: failure},
				{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
				{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			query:     Q(`suite:x,y`),
			expectErr: query.ErrNoDataForQuery{Q(`suite:x`)},
		},
		{ //////////////////////////////////////////////////////////////////////
			query:     Q(`suite:a,b:x`),
			expectErr: query.ErrNoDataForQuery{Q(`suite:a,b:x`)},
		},
	} {
		got, err := tree.Glob(test.query)
		if diff := cmp.Diff(err, test.expectErr); diff != "" {
			t.Errorf("Glob('%v') error: %v", test.query, err)
			continue
		}
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("Glob('%v'):\n%v", test.query, diff)
		}
	}
}

func TestFormat(t *testing.T) {
	type QueryData = query.QueryData[string]

	tree, err := NewTree(t,
		QueryData{Query: Q(`suite:*`), Data: skip},
		QueryData{Query: Q(`suite:a,*`), Data: failure},
		QueryData{Query: Q(`suite:a,b,*`), Data: failure},
		QueryData{Query: Q(`suite:a,b:c:d;*`), Data: failure},
		QueryData{Query: Q(`suite:a,b:c:d="e";*`), Data: failure},
		QueryData{Query: Q(`suite:h,b:c:f="g";*`), Data: abort},
		QueryData{Query: Q(`suite:a,b:c:f="g";*`), Data: skip},
		QueryData{Query: Q(`suite:a,b:d:*`), Data: failure},
	)
	if err != nil {
		t.Fatalf("NewTree() returned %v", err)
	}

	callA := fmt.Sprint(tree)
	callB := fmt.Sprint(tree)

	if diff := cmp.Diff(callA, callB); diff != "" {
		t.Errorf("Format():\n%v", diff)
	}
}
