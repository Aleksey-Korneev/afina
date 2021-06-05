#include <afina/coroutine/Engine.h>

// #include <setjmp.h>
# include <csetjmp>
# include <cassert>
#include <cstdio>
#include <cstring>

namespace Afina {
namespace Coroutine {

void Engine::Store(context &ctx)
{
    char *NewStackStart = nullptr;
    size_t NewStackLength = 0;
    char NewStackEnd = 0;
    ctx.StackEnd = &NewStackEnd;
    if (&NewStackEnd > ctx.StackStart) {
        NewStackStart = ctx.StackStart;
        NewStackLength = ctx.StackEnd - ctx.StackStart;
    } else {
        NewStackStart = ctx.StackEnd;
        NewStackLength = ctx.StackStart - ctx.StackEnd;
    }
    auto _size = std::get<1>(ctx.Stack);
    if (NewStackLength > _size || 2 * NewStackLength <= _size) {
        delete[] std::get<0>(ctx.Stack);
        // Recreate Stack buffer.
        std::get<0>(ctx.Stack) = new char[NewStackLength];
        std::get<1>(ctx.Stack) = NewStackLength;
    }
    // Fulfill new stack buffer.
    std::memcpy(std::get<0>(ctx.Stack), NewStackStart, NewStackLength);
}

void Engine::Restore(context &ctx)
{
    char address = 0;
    char *Begin = ctx.StackStart; // High
    char *End = ctx.StackEnd;     // Low
    // Can be confused.
    if (End > Begin) {
        Begin = ctx.StackEnd;
        End = ctx.StackStart;
    }
    // Why there was while?
    if (&address >= End && &address <= Begin) {
        Restore(ctx);
    }
    cur_routine = &ctx;
    // std::memcpy(ctx.Low, std::get<0>(ctx.Stack), size);
    std::memcpy(End, std::get<0>(ctx.Stack), Begin - End);
    longjmp(ctx.Environment, 1);
}

void Engine::yield()
{
    // No alive coroutines.
    if (alive == nullptr) {
        return;
    }
    // Already invoked and no more can be called.
    if (alive->next == nullptr && cur_routine == alive) {
        return;
    }
    context *next_routine = alive;
    if (cur_routine == alive) {
        next_routine = alive->next;
    }
    // TODO
    //enter(next_routine);
    Launch(next_routine);
}

void Engine::sched(void *routine_)
{
    if (routine_ == nullptr) {
        yield();
    }
    context *next_routine = static_cast<context *>(routine_);
    /*
    if (next_routine == nullptr) {
        this->yield();
    }
    */
    if (cur_routine == next_routine || next_routine->is_blocked) {
        return;
    }
    // TODO
    // enter(next_routine);
    Launch(next_routine);
}

/*
void Engine::enter(Engine::context *ctx)
{
    if (cur_routine != idle_ctx) {
        if (setjmp(cur_routine->Environment) > 0) {
            // TODO
            return;
        }
        Store(*cur_routine);
    }
    Restore(*ctx);
}
*/

void Engine::block(void *routine_) {
    context *coro = static_cast<context *>(routine_);
    if (routine_ == nullptr) {
        coro = cur_routine;
    }

    if (coro == nullptr) {
        return;
    }
    if (coro->is_blocked) {
        return;
    }

    // Skip this coroutine.
    if (coro->prev) {
        coro->prev->next = coro->next;
    }
    if (coro->next) {
        coro->next->prev = coro->prev;
    }

    if (coro == alive) {
        alive = alive->next;
    }

    coro->prev = nullptr;
    coro->next = blocked;

    // Move to beginning of blocked list or create it.
    if (blocked) {
        blocked->prev = coro;
    }
    blocked = coro;

    // Set blocked flag.
    coro->is_blocked = true;

    if (coro == cur_routine) {
        Launch(idle_ctx);
    }
}

void Engine::unblock(void *routine_) {
    context *coro = static_cast<context *>(routine_);
    // Coroutine doesn't exist or is already unblocked.
    if (coro == nullptr) {
        return;
    }
    if (!coro->is_blocked) {
        return;
    }

    // Skip this coroutine.
    if (coro->prev) {
        coro->prev->next = coro->next;
    }
    if (coro->next) {
        coro->next->prev = coro->prev;
    }

    // Delete from blocked list.
    if (coro == blocked) {
        blocked = blocked->next;
    }

    coro->prev = nullptr;
    coro->next = alive;
    // Move to beginning of alive list or create it.
    if (alive) {
        alive->prev = coro;
    }
    alive = coro;

    // Set blocked flag false.
    coro->is_blocked = false;
}

void Engine::Launch(context *ctx) {
    // Should I exit, throw exception or what?..
    // Should not happen.
    assert(ctx != cur_routine);
    if (ctx == nullptr) {
        return;
    }
    if (cur_routine != idle_ctx) {
        if (setjmp(cur_routine->Environment) > 0) {
            return;
        }
        Store(*cur_routine);
    }
    Restore(*ctx);
}

// Delete alive and blocked coroutines.
Engine::~Engine()
{
    context *coro = alive;
    for (; coro != nullptr; ) {
        context *coro_prev = coro;
        coro = coro->next;
        delete[] coro_prev;
    }

    coro = blocked;
    for (; coro != nullptr; ) {
        context *coro_prev = coro;
        coro = coro->next;
        delete[] coro_prev;
    }
}

} // namespace Coroutine
} // namespace Afina
