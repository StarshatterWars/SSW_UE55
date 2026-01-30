#pragma once

#include "CoreMinimal.h"

/**
 * Unreal-friendly Menu system.
 * - Uses FString
 * - Uses TArray for storage
 * - Menu owns MenuItem allocations
 */

class Menu;

class STARSHATTERWARS_API MenuItem
{
public:
    static const char* TYPENAME() { return "MenuItem"; }

    MenuItem() = default;

    MenuItem(const FString& InLabel, uintptr_t InData = 0, bool bInEnabled = true)
        : Text(InLabel)
        , Data(InData)
        , bEnabled(bInEnabled)
        , bSelected(false)
        , MenuPtr(nullptr)
        , SubMenuPtr(nullptr)
    {
    }

    ~MenuItem() = default;

    // Text
    const FString& GetText() const { return Text; }
    void SetText(const FString& InText) { Text = InText; }

    // Data (opaque)
    uintptr_t GetData() const { return Data; }
    void SetData(uintptr_t InData) { Data = InData; }

    // Flags
    bool IsEnabled() const { return bEnabled; }
    void SetEnabled(bool bInEnabled) { bEnabled = bInEnabled; }

    bool IsSelected() const { return bSelected; }
    void SetSelected(bool bInSelected) { bSelected = bInSelected; }

    // Links
    Menu* GetMenu() const { return MenuPtr; }
    void SetMenu(Menu* InMenu) { MenuPtr = InMenu; }

    Menu* GetSubmenu() const { return SubMenuPtr; }
    void SetSubmenu(Menu* InSubmenu) { SubMenuPtr = InSubmenu; }

    bool HasSubMenu() const { return SubMenuPtr != nullptr; }

private:
    FString   Text;
    uintptr_t Data = 0;

    bool bEnabled = true;
    bool bSelected = false;

    Menu* MenuPtr = nullptr;
    Menu* SubMenuPtr = nullptr;

    friend class Menu;
};


class STARSHATTERWARS_API Menu
{
public:
    static const char* TYPENAME() { return "Menu"; }

    Menu() = default;
    explicit Menu(const FString& InTitle) : Title(InTitle) {}

    virtual ~Menu()
    {
        ClearItems();
    }

    // Title
    const FString& GetTitle() const { return Title; }
    void SetTitle(const FString& InTitle) { Title = InTitle; }

    // Parent
    Menu* GetParent() const { return Parent; }
    void SetParent(Menu* InParent) { Parent = InParent; }

    // Items
    void AddItem(const FString& Label, uintptr_t Value = 0, bool bEnabled = true)
    {
        MenuItem* Item = new MenuItem(Label, Value, bEnabled);
        if (Item)
        {
            Item->MenuPtr = this;
            Items.Add(Item);
        }
    }

    void AddItem(MenuItem* Item)
    {
        if (!Item)
            return;

        if (Item->SubMenuPtr)
            Item->SubMenuPtr->SetParent(this);

        Item->MenuPtr = this;
        Items.Add(Item);
    }

    void AddMenu(const FString& Label, Menu* SubMenu, uintptr_t Value = 0)
    {
        MenuItem* Item = new MenuItem(Label, Value, true);
        if (Item)
        {
            Item->MenuPtr = this;
            Item->SubMenuPtr = SubMenu;

            if (SubMenu)
                SubMenu->Parent = this;

            Items.Add(Item);
        }
    }

    MenuItem* GetItem(int32 Index) const
    {
        return Items.IsValidIndex(Index) ? Items[Index] : nullptr;
    }

    void SetItem(int32 Index, MenuItem* Item)
    {
        if (Items.IsValidIndex(Index) && Item)
        {
            Items[Index] = Item;
            Item->MenuPtr = this;
        }
    }

    int32 NumItems() const
    {
        return Items.Num();
    }

    void ClearItems()
    {
        for (MenuItem* Item : Items)
        {
            delete Item;
        }
        Items.Reset();
    }

    const TArray<MenuItem*>& GetItems() const { return Items; }
    TArray<MenuItem*>& GetItems() { return Items; }

private:
    FString Title;
    TArray<MenuItem*> Items;
    Menu* Parent = nullptr;
};


class STARSHATTERWARS_API MenuHistory
{
public:
    static const char* TYPENAME() { return "MenuHistory"; }

    MenuHistory() = default;
    virtual ~MenuHistory()
    {
        History.Reset(); // does NOT delete menus; history is non-owning
    }

    Menu* GetCurrent() const
    {
        return History.Num() > 0 ? History.Last() : nullptr;
    }

    Menu* GetLevel(int32 N) const
    {
        return History.IsValidIndex(N) ? History[N] : nullptr;
    }

    Menu* Find(const FString& InTitle) const
    {
        for (Menu* M : History)
        {
            if (M && M->GetTitle().Equals(InTitle, ESearchCase::IgnoreCase))
                return M;
        }
        return nullptr;
    }

    void Pop()
    {
        if (History.Num() > 0)
            History.Pop(false);
    }

    void Push(Menu* InMenu)
    {
        if (InMenu)
            History.Add(InMenu);
    }

    void Clear()
    {
        History.Reset();
    }

private:
    TArray<Menu*> History; // non-owning
};
