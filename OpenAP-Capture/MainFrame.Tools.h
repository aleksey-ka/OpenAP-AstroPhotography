// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#pragma once

#include <QMainWindow>
#include <QPainter>

#include <Image.Math.Advanced.h>

namespace Ui {
    class MainFrame;
}

class Tools {
public:

    template<class T, class ...Args>
    T* New( Args... args )
    {
        Delete<T>();
        auto tool = std::make_shared<T>( ui, args... );
        tools.push_back( tool );
        return tool.get();
    }

    template<class T>
    bool Delete()
    {
        return doDelete( typeid( T ) );
    }

    template<class T>
    T* Toggle()
    {
        if( !Delete<T>() ) {
            auto tool = std::make_shared<T>( ui );
            tools.push_back( tool );
            return tool.get();
        }
        return 0;
    }

    template<class T>
    T* TryGet()
    {
        int pos = find( typeid( T ) );
        if( pos != -1 ) {
            return static_cast<T*>( tools[pos].get() );
        }
        return 0;
    }

    void OnNewImage( const CRawU16Image* );
    void Draw( QPixmap& );

    class TargetCircle;
    class FocusingHelper;

    Tools( Ui::MainFrame* _ui ) : ui( _ui ) { assert( ui != 0 ); }

private:
    class Base {
    public:
        Base( Ui::MainFrame* _ui ) : ui( _ui ) {}
        virtual ~Base() {}

        virtual void OnNewImage( const CRawU16Image* ) {}
        virtual void Draw( QPainter&, QPixmap& ) const {};

    protected:
        Ui::MainFrame* ui;
    };

    Ui::MainFrame* ui;
    std::vector<std::shared_ptr<Base>> tools;

    int find( const std::type_info& );
    bool doDelete(  const std::type_info& );
};

class Tools::TargetCircle : public Tools::Base {
public:
    using Base::Base;

    // Tools::Base
    virtual void Draw( QPainter& painter, QPixmap& pixmap ) const override;
};

class Tools::FocusingHelper : public Tools::Base {
public:
    using Base::Base;
    FocusingHelper( Ui::MainFrame* ui, int x, int y ) : Base( ui ), focusingHelper( std::make_shared<CFocusingHelper>( x, y ) ) {}

    CFocusingHelper* getFocusingHelper() { return focusingHelper.get(); }

    // Tools::Base
    virtual void Draw( QPainter& painter, QPixmap& ) const override;

private:
    std::shared_ptr<CFocusingHelper> focusingHelper = std::make_shared<CFocusingHelper>();
};
