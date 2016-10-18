/* 
 * Copyright (c) 2016 lymastee, All rights reserved.
 * Contact: lymastee@hotmail.com
 *
 * This file is part of the GSLIB project.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef bintree_cb1a3ccc_3e08_429b_bbeb_47e93ef74da1_h
#define bintree_cb1a3ccc_3e08_429b_bbeb_47e93ef74da1_h

#include <assert.h>
#include <gslib/pool.h>
#include <gslib/string.h>   /* test */

__gslib_begin__

struct _bintree_trait_copy {};
struct _bintree_trait_detach {};

template<class _ty>
struct _bintreenode_cpy_wrapper
{
    typedef _ty value;
    typedef _bintreenode_cpy_wrapper<_ty> myref;
    typedef _bintree_trait_copy tsf_behavior;

    value               _value;
    myref*              _left;
    myref*              _right;
    myref*              _parent;

    myref() { _left = _right = _parent = 0; }
    value* get_ptr() { return &_value; }
    const value* const_ptr() const { return &_value; }
    value& get_ref() { return _value; }
    const value& const_ref() const { return _value; }
    void born() {}
    void kill() {}
    template<class _cst>
    void born() {}
    template<class _cst>
    void kill() {}
    void copy(const myref* a) { get_ref() = a->const_ref(); }
    void attach(myref* a) { assert(0); }
};

template<class _ty>
struct _bintreenode_wrapper
{
    typedef _ty value;
    typedef _bintreenode_wrapper<_ty> myref;
    typedef _bintree_trait_detach tsf_behavior;

    value*              _value;
    myref*              _left;
    myref*              _right;
    myref*              _parent;

    myref()
    {
        _left = _right = _parent = 0;
        _value = 0;
    }
    value* get_ptr() { return _value; }
    const value* const_ptr() const { return _value; }
    value& get_ref() { return *_value; }
    const value& const_ref() const { return *_value; }
    void copy(const myref* a) { get_ref() = a->const_ref(); }
    //void born() { _value = gs_new(value); }
    void born() { !! }
    template<class _cst>
    void born() { _value = gs_new(_cst); }
    void kill() { if(_value) { gs_del(value, _value); _value = 0; } }
    template<class _cst>
    void kill() { if(_value) { gs_del(_cst, _value); _value = 0; } }
    void attach(myref* a)
    {
        assert(a && a->_value);
        kill();
        _value = a->_value;
        a->_value = 0;
    }
};

template<class _wrapper>
struct _bintree_allocator
{
    typedef _wrapper wrapper;
    static wrapper* born() { return gs_new(wrapper); }
    static void kill(wrapper* w) { gs_del(wrapper, w); }
};

template<class _val>
struct _bintreenode_val
{
    typedef _val value;
    typedef const _val const_value;
    union
    {
        value*          _vptr;
        const_value*    _cvptr;
    };
    value* get_wrapper() const { return _vptr; }
    operator bool() const { return _vptr != 0; }
    bool is_left() const { return (_cvptr && _cvptr->_parent) ? _cvptr->_parent->_left == _cvptr : false; }
    bool is_right() const { return (_cvptr && _cvptr->_parent) ? _cvptr->_parent->_right == _cvptr : false; }
    bool is_root() const { return _cvptr ? (!_cvptr->_parent) : false; }
    bool is_leaf() const { return _cvptr ? (!_cvptr->_left && !_cvptr->_right) : false; }
    int up_depth() const
    {
        int depth = 0;
        for(value* p = _vptr; p; p = p->_parent, depth ++);
        return depth;
    }
    int down_depth() const { return _down_depth(_vptr, 0); }
    bool operator==(const value* v) const { return _vptr == v; }
    bool operator!=(const value* v) const { return _vptr != v; }

public:
    static void connect_left_child(value* p, value* l)
    {
        assert(p && l && !p->_left && !l->_parent);
        p->_left = l;
        l->_parent = p;
    }
    static void connect_right_child(value* p, value* r)
    {
        assert(p && r && !p->_right && !r->_parent);
        p->_right = r;
        r->_parent = p;
    }
    static bool disconnect_parent_child(value* p, value* c)
    {
        assert(p && c && (c->_parent == p));
        if(p->_left == c) {
            p->_left = c->_parent = 0;
            return true;
        }
        assert(p->_right == c);
        p->_right = c->_parent = 0;
        return false;
    }

private:
    static int _down_depth(value* v, int ctr)
    {
        if(v == 0)
            return ctr;
        ctr ++;
        return gs_max(_down_depth(v->_left, ctr), 
            _down_depth(v->_right, ctr)
            );
    }

protected:
    value* vleft() const { return _vptr ? _vptr->_left : 0; }
    value* vright() const { return _vptr ? _vptr->_right : 0; }
    value* vparent() const { return _vptr ? _vptr->_parent : 0; }
    value* vsibling() const
    {
        if(!_vptr || !_vptr->_parent)
            return 0;
        if(is_left())
            return _vptr->_right;
        else if(is_right())
            return _vptr->_left;
        assert(!"unexpected.");
        return 0;
    }
    value* vroot() const
    {
        if(!_vptr)
            return 0;
        value* p = _vptr;
        for( ; p->_parent; p = p->_parent);
        return p;
    }

protected:
    template<class _lambda, class _value>
    static void preorder_traversal(_lambda lam, _value* v)
    {
        assert(v);
        lam(v);
        if(v->_left)
            preorder_traversal(lam, v->_left);
        if(v->_right)
            preorder_traversal(lam, v->_right);
    }
    template<class _lambda, class _value>
    static void inorder_traversal(_lambda lam, _value* v)
    {
        assert(v);
        if(v->_left)
            inorder_traversal(lam, v->_left);
        lam(v);
        if(v->_right)
            inorder_traversal(lam, v->_right);
    }
    template<class _lambda, class _value>
    static void postorder_traversal(_lambda lam, _value* v)
    {
        assert(v);
        if(v->_left)
            postorder_traversal(lam, v->_left);
        if(v->_right)
            postorder_traversal(lam, v->-right);
        lam(v);
    }

public:
    template<class _lambda>
    void inorder_traversal(_lambda lam) { if(_vptr) inorder_traversal(lam, _vptr); }
    template<class _lambda>
    void inorder_traversal(_lambda lam) const { if(_cvptr) inorder_traversal(lam, _cvptr); }
    template<class _lambda>
    void preorder_traversal(_lambda lam) { if(_vptr) preorder_traversal(lam, _vptr); }
    template<class _lambda>
    void preorder_traversal(_lambda lam) const { if(_cvptr) preorder_traversal(lam, _cvptr); }
    template<class _lambda>
    void postorder_traversal(_lambda lam) { if(_vptr) postorder_traversal(lam, _vptr); }
    template<class _lambda>
    void postorder_traversal(_lambda lam) const { if(_cvptr) postorder_traversal(lam, _cvptr); }
};

template<class _ty,
    class _wrapper = _bintreenode_cpy_wrapper<_ty> >
class _bintree_const_iterator:
    public _bintreenode_val<_wrapper>
{
public:
    typedef _ty value;
    typedef _wrapper wrapper;
    typedef _bintree_const_iterator<_ty, _wrapper> iterator;

public:
    iterator(const wrapper* w = 0) { _cvptr = w; }
    bool is_valid() const { return _cvptr != 0; }
    const value* get_ptr() const { return _cvptr->const_ptr(); }
    const value* operator->() const { return _cvptr->const_ptr(); }
    const value& operator*() const { return _cvptr->const_ref(); }
    iterator left() const { return iterator(vleft()); }
    iterator right() const { return iterator(vright()) }
    iterator parent() const { return iterator(vparent()); }
    iterator sibling() const { return iterator(vsibling()); }
    iterator root() const { return iterator(vroot()); }
    bool operator==(const iterator& that) const { return _cvptr == that._cvptr; }
    bool operator!=(const iterator& that) const { return _cvptr != that._cvptr; }
};

template<class _ty,
    class _wrapper = _bintreenode_cpy_wrapper<_ty> >
class _bintree_iterator:
    public _bintree_const_iterator<_ty, _wrapper>
{
public:
    typedef _ty value;
    typedef _wrapper wrapper;
    typedef _bintree_const_iterator<_ty, _wrapper> const_iterator;
    typedef _bintree_const_iterator<_ty, _wrapper> superref;
    typedef _bintree_iterator<_ty, _wrapper> iterator;

public:
    iterator(wrapper* w): superref(w) {}
    value* get_ptr() const { return _vptr->get_ptr(); }
    value* operator->() const { return _vptr->get_ptr(); }
    value& operator*() const { return _vptr->get_ref(); }
    bool operator==(const iterator& that) const { return _vptr == that._vptr; }
    bool operator!=(const iterator& that) const { return _vptr != that._vptr; }
    bool operator==(const const_iterator& that) const { return _vptr == that._vptr; }
    bool operator!=(const const_iterator& that) const { return _vptr != that._vptr; }
    operator const_iterator() { return const_iterator(_cvptr); }
    void to_root() { _vptr = vroot(); }
    void to_left() { _vptr = vleft(); }
    void to_right() { _vptr = vright(); }
    void to_sibling() { _vptr = vsibling(); }
    void to_parent() { _vptr = vparent(); }
    iterator left() const { return iterator(vleft()); }
    iterator right() const { return iterator(vright()); }
    iterator parent() const { return iterator(vparent()); }
    iterator sibling() const { return iterator(vsibling()); }
    iterator root() const { return iterator(vroot()); }
};

template<class _ty,
    class _wrapper = _bintreenode_cpy_wrapper<_ty>,
    class _alloc = _bintree_allocator<_wrapper> >
class bintree:
    public _bintreenode_val<_wrapper>
{
public:
    typedef _ty value;
    typedef _wrapper wrapper;
    typedef _alloc alloc;
    typedef bintree<value, wrapper, alloc> myref;
    typedef _bintree_const_iterator<_ty, _wrapper> const_iterator;
    typedef _bintree_iterator<_ty, _wrapper> iterator;

public:
    bintree() { _vptr = 0; }
    ~bintree() { destroy(); }
    void destroy() { erase(get_root()); }
    void clear() { destroy(); }
    void erase(iterator i)
    {
        if(!i.is_valid())
            return;
        if(i.is_left())
            i.parent().get_wrapper()->_left = 0;
        else if(i.is_right())
            i.parent().get_wrapper()->_right = 0;
        if(is_root(i))
            _vptr = 0;
        _erase(i);
    }
    void adopt(wrapper* w)
    {
        assert(!_vptr && "use attach method.");
        _vptr = w;
    }
    iterator get_root() { return iterator(_vptr); }
    const_iterator const_root() const { return const_iterator(_cvptr); }
    bool is_root(iterator i) const { return i.is_valid() ? (_cvptr == i.get_wrapper()) : false; }
    bool is_valid() const { return _cvptr != 0; }
    bool is_mine(iterator i) const
    {
        if(!i.is_valid())
            return false;
        i.to_root();
        return i.get_wrapper() == _vptr;
    }
    int depth() const { return _cvptr->down_depth(); }
    void swap(myref& that) { gs_swap(_vptr, that._vptr); }
    enum _bintree_pos
    {
        btp_left,
        btp_right,
        btp_parent,
    };
    template<class _cst = value>
    iterator insert(iterator i, _bintree_pos ipos, _bintree_pos cpos)
    {
        if(!i.is_valid())
            return !is_valid() ? _init<_cst>() : iterator(0);
        wrapper* n = alloc::born();
        n->born<_cst>();
        n->_parent = i.get_wrapper();
        iterator old = i;   /* find the old one */
        switch(ipos)
        {
        case btp_left:
            old.to_left();
            i.get_wrapper()->_left = n;
            break;
        case btp_right:
            old.to_right();
            i.get_wrapper()->_right = n;
            break;
        }
        switch(cpos)
        {
        case btp_left:
            n->_left = old.get_wrapper();
            break;
        case btp_right:
            n->_right = old.get_wrapper();
            break;
        }
        if(old.is_valid())
            old.get_wrapper()->_parent = n;
        return iterator(n);
    }
    template<class _cst = value>
    iterator insertll(iterator i) { return insert<_cst>(i, btp_left, btp_left); }
    template<class _cst = value>
    iterator insertlr(iterator i) { return insert<_cst>(i, btp_left, btp_right); }
    template<class _cst = value>
    iterator insertrl(iterator i) { return insert<_cst>(i, btp_right, btp_left); }
    template<class _cst = value>
    iterator insertrr(iterator i) { return insert<_cst>(i, btp_right, btp_right); }

    /* The detach and attach methods, provide subtree operations */
    myref& detach(myref& subtree, iterator i)
    {
        assert(i && is_mine(i));
        if(subtree.is_valid())
            subtree.destroy();
        detach<myref>(subtree, i);
        return subtree;
    }
    template<class _cont>
    void detach(_cont& cont)
    {
        cont.adopt(_vptr);
        _vptr = 0;
    }
    template<class _cont>
    void detach(_cont& cont, iterator i)
    {
        assert(i && is_mine(i));
        if(i == get_root())
            return detach(cont);
        iterator p = i.parent();
        assert(p);
        disconnect_parent_child(p.get_wrapper(), i.get_wrapper());
        cont.adopt(i.get_wrapper());
    }

    /* different transform functions */
    template<class _tsftrait>
    void transform(wrapper* src, wrapper* des);
    template<>
    void transform<_bintree_trait_copy>(wrapper* src, wrapper* des)
    {
        assert(src && des);
        des->copy(src);
        des->_left = src->_left;
        des->_right = src->_right;
        src->_left = 0;
        src->_right = 0;
    }
    template<>
    void transform<_bintree_trait_detach>(wrapper* src, wrapper* des)
    {
        assert(src && des);
        des->kill();
        des->attach(src);
        des->_left = src->_left;
        des->_right = src->_right;
        src->_left = 0;
        src->_right = 0;
    }
    void attach(myref& subtree, iterator i)
    {
        assert(i && is_mine(i) && i.is_leaf());
        if(i.is_root()) {
            swap(subtree);
            return;
        }
        iterator p = i.parent();
        assert(p);
        bool leftp = disconnect_parent_child(p.get_wrapper(), i.get_wrapper());
        gs_swap(subtree._vptr, i._vptr);
        leftp ? connect_left_child(p.get_wrapper(), i.get_wrapper()) :
            connect_right_child(p.get_wrapper(), i.get_wrapper());
        subtree.destroy();
    }

public:
    template<class _lambda>
    void preorder_for_each(_lambda lam) { preorder_traversal([](wrapper* w) { lam(w->get_ptr()); }); }
    template<class _lambda>
    void preorder_const_for_each(_lambda lam) const { preorder_traversal([](const wrapper* w) { lam(w->const_ptr()); }); }
    template<class _lambda>
    void inorder_for_each(_lambda lam) { inorder_traversal([](wrapper* w) { lam(w->get_ptr()); }); }
    template<class _lambda>
    void inorder_const_for_each(_lambda lam) const { inorder_traversal([](const wrapper* w) { lam(w->const_ptr()); }); }
    template<class _lambda>
    void postorder_for_each(_lambda lam) { postorder_traversal([](wrapper* w) { lam(w->get_ptr()); }); }
    template<class _lambda>
    void postorder_const_for_each(_lambda lam) const { postorder_traversal([](const wrapper* w) { lam(w->const_ptr()); }); }

protected:
    bintree(iterator i) { _vptr = i._vptr; }
    void _erase(iterator i)
    {
        if(!i.is_valid())
            return;
        _erase(i.left());
        _erase(i.right());
        wrapper* w = i.get_wrapper();
        w->kill();
        alloc::kill(w);
    }
    template<class _cst>
    iterator _init()
    {
        _vptr = alloc::born();
        _vptr->born<_cst>();
        return iterator(_vptr);
    }

public:
    bool debug_check(iterator i)
    {
        iterator l = i.left(), r = i.right();
        if(l.is_valid() && l.parent() != i) {
            assert(0);
            return false;
        }
        if(r.is_valid() && r.parent() != i) {
            assert(0);
            return false;
        }
        if(l.is_valid() && !debug_check(l))
            return false;
        if(r.is_valid() && !debug_check(r))
            return false;
        return true;
    }
};

/* for test only, print the tree. */
template<class _bintree>
class _print_bintree
{
public:
    typedef typename _bintree::value value;
    typedef typename _bintree::iterator iterator;
    typedef typename _bintree::const_iterator const_iterator;

protected:
    _bintree&       _inst;
    string          _src;

public:
    _print_bintree(_bintree& b):_inst(b) {}
    const gchar* print()
    {
        _src.clear();
        append(_inst.get_root(), 0);
        return _src.c_str();
    }

protected:
    string decorate(iterator i, int level)
    {
        if(!i.is_valid())
            return string();
        string r, a;
        for(int j = 0; j < level; j ++)
            r.append(_t("  "));
        i.is_leaf() ? r.append(_t("+ ")) : r.append(_t("- "));
        if(i.is_root())
            a = _t("root(");
        else if(i.is_left())
            a.format(1024, _t("left%d("), level);
        else if(i.is_right())
            a.format(1024, _t("right%d("), level);
        else {
            /* inner error? */
            assert(0);
        }
        r.append(a);
        r.append(i->to_string());
        r.append(_t(")\n"));
        return r;
    }
    void append(iterator i, int level)
    {
        if(!i.is_valid())
            return;
        _src.append(decorate(i, level).c_str());
        level ++;
        append(i.left(), level);
        append(i.right(), level);
    }
};

__gslib_end__

#endif