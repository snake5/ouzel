// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

namespace ouzel
{
    class Application: public Noncopyable, public ReferenceCounted, public EventHandler
    {
    public:
        Application(Engine* engine);
        virtual ~Application();
        
        Engine* getEngine() const { return _engine; }
        
        virtual bool handleEvent(const Event& event) override;
        virtual void update(float delta) override;
        
    protected:
        Engine* _engine;
    };
}