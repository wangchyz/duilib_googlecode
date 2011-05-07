#ifndef __UIDELEGATE_H__
#define __UIDELEGATE_H__

#pragma once

namespace DuiLib {

class UILIB_API CDelegateBase	 
{
public:
    CDelegateBase(void* pObject, void* pFn);
    CDelegateBase(const CDelegateBase& rhs);
    virtual ~CDelegateBase();
    bool Equals(const CDelegateBase& rhs) const;
    bool operator() (TEventUI& event);
    virtual CDelegateBase* Copy() = 0;

protected:
    void* GetFn();
    void* GetObject();
    virtual bool Invoke(TEventUI& event) = 0;

private:
    void* m_pObject;
    void* m_pFn;
};

class CDelegateStatic: public CDelegateBase
{
    typedef bool (*Fn)(TEventUI&);
public:
    CDelegateStatic(Fn pFn) : CDelegateBase(NULL, pFn) { } 
    CDelegateStatic(const CDelegateStatic& rhs) : CDelegateBase(rhs) { } 
    virtual CDelegateBase* Copy() { return new CDelegateStatic(*this); }

protected:
    virtual bool Invoke(TEventUI& event)
    {
        Fn pFn = (Fn)GetFn();
        return (*pFn)(event); 
    }
};

template <class O, class T>
class CDelegate : public CDelegateBase
{
    typedef bool (T::* Fn)(TEventUI&);
public:
    CDelegate(O* pObj, Fn pFn) : CDelegateBase(pObj, &pFn), m_pFn(pFn) { }
    CDelegate(const CDelegate& rhs) : CDelegateBase(rhs) { m_pFn = rhs.m_pFn; } 
    virtual CDelegateBase* Copy() { return new CDelegate(*this); }

protected:
    virtual bool Invoke(TEventUI& event)
    {
        O* pObject = (O*) GetObject();
        return (pObject->*m_pFn)(event); 
    }  

private:
    Fn m_pFn;
};

template <class O, class T>
CDelegate<O, T> MakeDelegate(O* pObject, bool (T::* pFn)(TEventUI&))
{
    return CDelegate<O, T>(pObject, pFn);
}

inline CDelegateStatic MakeDelegate(bool (*pFn)(TEventUI&))
{
    return CDelegateStatic(pFn); 
}

class UILIB_API CEventSource
{
    typedef bool (*FnType)(TEventUI&);
public:
    ~CEventSource();
    void operator+= (CDelegateBase& d);
    void operator+= (FnType pFn);
    void operator-= (CDelegateBase& d);
    void operator-= (FnType pFn);
    bool operator() (TEventUI& event);

protected:
    CStdPtrArray m_aDelegates;
};

} // namespace DuiLib

#endif // __UIDELEGATE_H__