#include <afina/coroutine/Engine.h>

#include <setjmp.h>
#include <stdio.h>
#include <cstring>

namespace Afina {
namespace Coroutine {

void Engine::Store(context &ctx)
{
    char address;
    if (&address > ctx.Low) {
        ctx.High = &address;
    } else {
        ctx.Low = &address;
    }
    auto stack_size = ctx.High - ctx.Low;
    auto item = std::get<1>(ctx.Stack);
    if (2 * stack_size > item || stack_size > item) {
        delete[] std::get<0>(ctx.Stack);
        std::get<0>(ctx.Stack) = new char[stack_size];
        std::get<1>(ctx.Stack) = stack_size;
    }
    std::memcpy(std::get<0>(ctx.Stack), ctx.Low, stack_size);

}

void Engine::Restore(context &ctx)
{
    char address;
    while (&address >= ctx.Low && &address <= ctx.High) {
        this->Restore(ctx);
    }
    auto stack_size = ctx.High - ctx.Low;
    std::memcpy(ctx.Low, std::get<0>(ctx.Stack), size);
    cur_routine = &ctx;
    longjmp(ctx.Environment, 1);
}

void Engine::yield()
{
    if (!alive) {
        // TODO
        return;
    }
    if (cur_routine == alive && !(alive->next)) {
        // TODO
        return;
    }

    context *next_routine = alive;
    if (alive == cur_routine) {
        next_routine = alive->next;
    }
    // TODO
    enter(next_routine);
}

void Engine::sched(void *routine_)
{
    auto next_routine = static_cast<context *>(routine_);
    if (next_routine == nullptr) {
        this->yield();
    }
    if (next_routine->is_blocked || cur_routine == next_routine) {
        // TODO
        return;
    }
    // TODO
    enter(next_routine);
}

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

} // namespace Coroutine
} // namespace Afina
