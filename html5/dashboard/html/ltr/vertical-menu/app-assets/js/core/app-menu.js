/*=========================================================================================
  File Name: app-menu.js
  Description: Menu navigation, custom scrollbar, hover scroll bar, multilevel menu
  initialization and manipulations
  ----------------------------------------------------------------------------------------
  Item Name: Crypto ICO - Cryptocurrency Website Landing Page HTML + Dashboard Template
  Version: 1.0
  Author: Pixinvent
  Author URL: hhttp://www.themeforest.net/user/pixinvent
==========================================================================================*/
(function(window, document, $) {
  'use strict';

  $.app = $.app || {};

  var $body       = $('body');
  var $window     = $( window );

  // Main menu
  $.app.menu = {
    expanded: null,
    collapsed: null,
    hidden : null,
    container: null,
    horizontalMenu: false,

    manualScroller: {
      obj: null,

      init: function() {
        var scroll_theme = ($('.main-menu').hasClass('menu-dark')) ? 'light' : 'dark';
        this.obj = $(".main-menu-content").perfectScrollbar({
          suppressScrollX: true,
          theme: scroll_theme
        });
      },

      update: function() {
        if (this.obj) {
          // Scroll to currently active menu on page load if data-scroll-to-active is true
          if($('.main-menu').data('scroll-to-active') === true){
              var position;
              if( $(".main-menu-content").find('li.active').parents('li').length > 0 ){
                position = $(".main-menu-content").find('li.active').parents('li').last().position();
              }
              else{
                position = $(".main-menu-content").find('li.active').position();
              }
              setTimeout(function(){
                // $.app.menu.container.scrollTop(position.top);
                if(position !== undefined){
                  $.app.menu.container.stop().animate({scrollTop:position.top}, 300);
                }
                $('.main-menu').data('scroll-to-active', 'false');
              },300);
          }
          $(".main-menu-content").perfectScrollbar('update');
        }
      },

      enable: function() {
        this.init();
      },

      disable: function() {
        if (this.obj) {
          $('.main-menu-content').perfectScrollbar('destroy');
        }
      },

      updateHeight: function(){
        if( ($body.data('menu') == 'vertical-menu' || $body.data('menu') == 'vertical-menu-modern' || $body.data('menu') == 'vertical-overlay-menu' ) && $('.main-menu').hasClass('menu-fixed')){
          $('.main-menu-content').css('height', $(window).height() - $('.header-navbar').height() - $('.main-menu-header').outerHeight() - $('.main-menu-footer').outerHeight() );
          this.update();
        }
      }
    },

    init: function(compactMenu) {
      if($('.main-menu-content').length > 0){
        this.container = $('.main-menu-content');

        var menuObj = this;
        var defMenu = '';

        if(compactMenu === true){
          defMenu = 'collapsed';
        }

        this.change(defMenu);
      }
    },

    change: function(defMenu) {
      var currentBreakpoint = Unison.fetch.now(); // Current Breakpoint

      this.reset();

      var menuType = $body.data('menu');

      if (currentBreakpoint) {
        switch (currentBreakpoint.name) {
          case 'xl':
          case 'lg':
            if(menuType === 'vertical-overlay-menu'){
              this.hide();
            }
            else if(menuType === 'vertical-compact-menu'){
              this.open();
            }
            else{
              if(defMenu === 'collapsed')
                this.collapse(defMenu);
              else
                this.expand();
            }
            break;
          case 'md':
            if(menuType === 'vertical-overlay-menu'){
              this.hide();
            }
            else if(menuType === 'vertical-compact-menu'){
              this.open();
            }
            else{
              this.collapse();
            }
            break;
          case 'sm':
            this.hide();
            break;
          case 'xs':
            this.hide();
            break;
        }
      }

      // On the small and extra small screen make them overlay menu
      if(menuType === 'vertical-compact-menu'){
        this.toOverlayMenu(currentBreakpoint.name);
      }


      // Added data attribute brand-center for navbar-brand-center
      // TODO:AJ: Shift this feature in PUG.
      if($('.header-navbar').hasClass('navbar-brand-center')){
        $('.header-navbar').attr('data-nav','brand-center');
      }
      if(currentBreakpoint.name == 'sm' || currentBreakpoint.name == 'xs'){
        $('.header-navbar[data-nav=brand-center]').removeClass('navbar-brand-center');
      }else{
        $('.header-navbar[data-nav=brand-center]').addClass('navbar-brand-center');
      }

      // Dropdown submenu on small screen on click
      // --------------------------------------------------
      $('ul.dropdown-menu [data-toggle=dropdown]').on('click', function(event) {
        if($(this).siblings('ul.dropdown-menu').length > 0){
          event.preventDefault();
        }
        event.stopPropagation();
        $(this).parent().siblings().removeClass('show');
        $(this).parent().toggleClass('show');
      });
    },

    transit: function(callback1, callback2) {
      var menuObj = this;
      $body.addClass('changing-menu');

      callback1.call(menuObj);

      if($body.hasClass('vertical-layout')){
        if($body.hasClass('menu-open') || $body.hasClass('menu-expanded')){
          $('.menu-toggle').addClass('is-active');

          // Show menu header search when menu is normally visible
          if( $body.data('menu') === 'vertical-menu' || $body.data('menu') === 'vertical-content-menu'){
            if($('.main-menu-header')){
              $('.main-menu-header').show();
            }
          }
        }
        else{
          $('.menu-toggle').removeClass('is-active');

          // Hide menu header search when only menu icons are visible
          if( $body.data('menu') === 'vertical-menu' || $body.data('menu') === 'vertical-content-menu'){
            if($('.main-menu-header')){
              $('.main-menu-header').hide();
            }
          }
        }
      }

      setTimeout(function() {
        callback2.call(menuObj);
        $body.removeClass('changing-menu');

        menuObj.update();
      }, 500);
    },

    open: function() {
      this.transit(function() {
        $body.removeClass('menu-hide menu-collapsed').addClass('menu-open');
        this.hidden = false;
        this.expanded = true;
      }, function() {
        if(!$('.main-menu').hasClass('menu-native-scroll') && $('.main-menu').hasClass('menu-fixed') ){
          this.manualScroller.enable();
          $('.main-menu-content').css('height', $(window).height() - $('.header-navbar').height() - $('.main-menu-header').outerHeight() - $('.main-menu-footer').outerHeight() );
          // this.manualScroller.update();
        }
      });
    },

    hide: function() {

      this.transit(function() {
        $body.removeClass('menu-open menu-expanded').addClass('menu-hide');
        this.hidden = true;
        this.expanded = false;
      }, function() {
        if(!$('.main-menu').hasClass('menu-native-scroll') && $('.main-menu').hasClass('menu-fixed')){
          this.manualScroller.enable();
        }
      });
    },

    expand: function() {
      if (this.expanded === false) {
        if( $body.data('menu') == 'vertical-menu-modern' ){
          $('.modern-nav-toggle').find('.toggle-icon')
          .removeClass('ft-toggle-left').addClass('ft-toggle-right');

          // Code for localStorage
          if (typeof(Storage) !== "undefined") {
            localStorage.setItem("menuLocked", "true");
          }
        }
        /*if( $body.data('menu') == 'vertical-menu' || $body.data('menu') == 'vertical-menu-modern'){
          this.changeLogo('expand');
        }*/
        this.transit(function() {
          $body.removeClass('menu-collapsed').addClass('menu-expanded');
          this.collapsed = false;
          this.expanded = true;

        });
      }
    },

    collapse: function(defMenu) {
      if (this.collapsed === false) {
        if( $body.data('menu') == 'vertical-menu-modern' ){
          $('.modern-nav-toggle').find('.toggle-icon')
          .removeClass('ft-toggle-right').addClass('ft-toggle-left');

          // Code for localStorage
          if (typeof(Storage) !== "undefined") {
            localStorage.setItem("menuLocked", "false");
          }
        }
        this.transit(function() {
          $body.removeClass('menu-expanded').addClass('menu-collapsed');
          this.collapsed = true;
          this.expanded  = false;

        });
      }
    },

    toOverlayMenu: function(screen){
      var menu = $body.data('menu');
      if(screen == 'sm' || screen == 'xs'){
        if($body.hasClass(menu)){
          $body.removeClass(menu).addClass('vertical-overlay-menu');
        }
        if(menu == 'vertical-content-menu'){
          $('.main-menu').addClass('menu-fixed');
        }
      }
      else{
        if($body.hasClass('vertical-overlay-menu')){
          $body.removeClass('vertical-overlay-menu').addClass(menu);
        }
        if(menu == 'vertical-content-menu'){
          $('.main-menu').removeClass('menu-fixed');
        }
      }
    },

    toggle: function() {
      var currentBreakpoint = Unison.fetch.now(); // Current Breakpoint
      var collapsed = this.collapsed;
      var expanded = this.expanded;
      var hidden = this.hidden;
      var menu = $body.data('menu');

      switch (currentBreakpoint.name) {
        case 'xl':
        case 'lg':
        case 'md':
          if(expanded === true){
            if(menu == 'vertical-compact-menu' || menu == 'vertical-overlay-menu'){
              this.hide();
            }
            else{
              this.collapse();
            }
          }
          else{
            if(menu == 'vertical-compact-menu' || menu == 'vertical-overlay-menu'){
              this.open();
            }
            else{
              this.expand();
            }
          }
          break;
        case 'sm':
          if (hidden === true) {
            this.open();
          } else {
            this.hide();
          }
          break;
        case 'xs':
          if (hidden === true) {
            this.open();
          } else {
            this.hide();
          }
          break;
      }
    },

    update: function() {
      this.manualScroller.update();
    },

    reset: function() {
      this.expanded  = false;
      this.collapsed = false;
      this.hidden    = false;
      $body.removeClass('menu-hide menu-open menu-collapsed menu-expanded');
    },
  };

  // Navigation Menu
  $.app.nav = {
    container: $('.navigation-main'),
    initialized : false,
    navItem: $('.navigation-main').find('li').not('.navigation-category'),

    config: {
      speed: 300,
    },

    init: function(config) {
      this.initialized = true; // Set to true when initialized
      $.extend(this.config, config);

      this.bind_events();
    },

    bind_events: function() {
      var menuObj = this;

      $('.navigation-main').on('mouseenter.app.menu', 'li', function() {
        var $this = $(this);
        $('.hover', '.navigation-main').removeClass('hover');
        if( ($body.hasClass('menu-collapsed') && $body.data('menu') != 'vertical-menu-modern') || ($body.data('menu') == 'vertical-compact-menu' && !$body.hasClass('vertical-overlay-menu')) ){
          $('.main-menu-content').children('span.menu-title').remove();
          $('.main-menu-content').children('a.menu-title').remove();
          $('.main-menu-content').children('ul.menu-content').remove();

          // Title
          var menuTitle = $this.find('span.menu-title').clone(),
          tempTitle,
          tempLink;
          if(!$this.hasClass('has-sub') ){
            tempTitle = $this.find('span.menu-title').text();
            tempLink = $this.children('a').attr('href');
            if(tempTitle !== ''){
              menuTitle = $("<a>");
              menuTitle.attr("href", tempLink);
              menuTitle.attr("title", tempTitle);
              menuTitle.text(tempTitle);
              menuTitle.addClass("menu-title");
            }
          }
          // menu_header_height = ($('.main-menu-header').length) ? $('.main-menu-header').height() : 0,
          // fromTop = menu_header_height + $this.position().top + parseInt($this.css( "border-top" ),10);
          var fromTop;
          if($this.css( "border-top" )){
            fromTop = $this.position().top + parseInt($this.css( "border-top" ), 10);
          }
          else{
            fromTop = $this.position().top;
          }
          if($body.data('menu') !== 'vertical-compact-menu'){
            menuTitle.appendTo('.main-menu-content').css({
              position:'fixed',
              top : fromTop,
            });
          }

          // Content
          if($this.hasClass('has-sub') && $this.hasClass('nav-item')) {
            var menuContent = $this.children('ul:first');
            menuObj.adjustSubmenu($this);
          }
        }
        $this.addClass('hover');
      }).on('mouseleave.app.menu', 'li', function() {
        // $(this).removeClass('hover');
      }).on('active.app.menu', 'li', function(e) {
        $(this).addClass('active');
        e.stopPropagation();
      }).on('deactive.app.menu', 'li.active', function(e) {
        $(this).removeClass('active');
        e.stopPropagation();
      }).on('open.app.menu', 'li', function(e) {

        var $listItem = $(this);
        $listItem.addClass('open');

        menuObj.expand($listItem);

        // If menu collapsible then do not take any action
        if ($('.main-menu').hasClass('menu-collapsible')) {
          return false;
        }
        // If menu accordion then close all except clicked once
        else{
          $listItem.siblings('.open').find('li.open').trigger('close.app.menu');
          $listItem.siblings('.open').trigger('close.app.menu');
        }

        e.stopPropagation();
      }).on('close.app.menu', 'li.open', function(e) {
        var $listItem = $(this);

        $listItem.removeClass('open');
        menuObj.collapse($listItem);

        e.stopPropagation();
      }).on('click.app.menu', 'li', function(e) {
        var $listItem = $(this);
        if($listItem.is('.disabled')){
          e.preventDefault();
        }
        else{
          if( ($body.hasClass('menu-collapsed') && $body.data('menu') != 'vertical-menu-modern')  || ($body.data('menu') == 'vertical-compact-menu' && $listItem.is('.has-sub') && !$body.hasClass('vertical-overlay-menu'))){
            e.preventDefault();
          }
          else{
            if ($listItem.has('ul')) {
              if ($listItem.is('.open')) {
                $listItem.trigger('close.app.menu');
              } else {
                $listItem.trigger('open.app.menu');
              }
            } else {
              if (!$listItem.is('.active')) {
                $listItem.siblings('.active').trigger('deactive.app.menu');
                $listItem.trigger('active.app.menu');
              }
            }
          }
        }

        e.stopPropagation();
      });


      $('.navbar-header, .main-menu').on('mouseenter',modernMenuExpand).on('mouseleave',modernMenuCollapse);

      function modernMenuExpand(){
        if( $body.data('menu') == 'vertical-menu-modern'){
          $('.main-menu, .navbar-header').addClass('expanded');
          if($body.hasClass('menu-collapsed')){
            var $listItem = $('.main-menu li.menu-collapsed-open'),
            $subList = $listItem.children('ul');

            $subList.hide().slideDown(200, function() {
                $(this).css('display', '');
            });

            $listItem.addClass('open').removeClass('menu-collapsed-open');
            // $.app.menu.changeLogo('expand');
          }
        }
      }

      function modernMenuCollapse(){
        if($body.hasClass('menu-collapsed') && $body.data('menu') == 'vertical-menu-modern'){
          setTimeout(function(){
            if($('.main-menu:hover').length === 0 && $('.navbar-header:hover').length === 0){

              $('.main-menu, .navbar-header').removeClass('expanded');
              if($body.hasClass('menu-collapsed')){
                var $listItem = $('.main-menu li.open'),
                $subList = $listItem.children('ul');
                $listItem.addClass('menu-collapsed-open');

                $subList.show().slideUp(200, function() {
                    $(this).css('display', '');
                });

                $listItem.removeClass('open');
                // $.app.menu.changeLogo();
              }
            }
          },1);
        }
      }

      $('.main-menu-content').on('mouseleave', function(){
        if( $body.hasClass('menu-collapsed') || $body.data('menu') == 'vertical-compact-menu' ){
          $('.main-menu-content').children('span.menu-title').remove();
          $('.main-menu-content').children('a.menu-title').remove();
          $('.main-menu-content').children('ul.menu-content').remove();
        }
        $('.hover', '.navigation-main').removeClass('hover');
      });

      // If list item has sub menu items then prevent redirection.
      $('.navigation-main li.has-sub > a').on('click',function(e){
        e.preventDefault();
      });

      $('ul.menu-content').on('click', 'li', function(e) {
        var $listItem = $(this);
        if($listItem.is('.disabled')){
          e.preventDefault();
        }
        else{
          if ($listItem.has('ul')) {
            if ($listItem.is('.open')) {
              $listItem.removeClass('open');
              menuObj.collapse($listItem);
            } else {
              $listItem.addClass('open');

              menuObj.expand($listItem);

              // If menu collapsible then do not take any action
              if ($('.main-menu').hasClass('menu-collapsible')) {
                return false;
              }
              // If menu accordion then close all except clicked once
              else{
                $listItem.siblings('.open').find('li.open').trigger('close.app.menu');
                $listItem.siblings('.open').trigger('close.app.menu');
              }

              e.stopPropagation();
            }
          } else {
            if (!$listItem.is('.active')) {
              $listItem.siblings('.active').trigger('deactive.app.menu');
              $listItem.trigger('active.app.menu');
            }
          }
        }

        e.stopPropagation();
      });
    },

    /**
     * Ensure an admin submenu is within the visual viewport.
     * @param {jQuery} $menuItem The parent menu item containing the submenu.
     */
    adjustSubmenu: function ( $menuItem ) {
      var menuHeaderHeight, menutop, topPos, winHeight,
      bottomOffset, subMenuHeight, popOutMenuHeight, borderWidth, scroll_theme,
      $submenu = $menuItem.children('ul:first'),
      ul = $submenu.clone(true);

      menuHeaderHeight = $('.main-menu-header').height();
      menutop          = $menuItem.position().top;
      winHeight        = $window.height();
      borderWidth      = 0;
      subMenuHeight    = $submenu.height();

      if(parseInt($menuItem.css( "border-top" ),10) > 0){
        borderWidth = parseInt($menuItem.css( "border-top" ),10);
      }

      popOutMenuHeight = winHeight - menutop - $menuItem.height() - 30;
      scroll_theme     = ($('.main-menu').hasClass('menu-dark')) ? 'light' : 'dark';

      if($body.data('menu') === 'vertical-compact-menu'){
        topPos = menutop + borderWidth;
        popOutMenuHeight = winHeight - menutop - 30;
      }
      else if($body.data('menu') === 'vertical-content-menu'){
        topPos = menutop + $menuItem.height() + borderWidth;
        popOutMenuHeight = winHeight - $('.content-header').height() -$menuItem.height() - menutop - 60;
      }
      else{
        topPos = menutop + $menuItem.height() + borderWidth;
      }
      
      if($body.data('menu') == 'vertical-content-menu'){
        ul.addClass('menu-popout').appendTo('.main-menu-content').css({
          'top' : topPos,
          'position' : 'fixed',
        });
      }
      else{
        ul.addClass('menu-popout').appendTo('.main-menu-content').css({
          'top' : topPos,
          'position' : 'fixed',
          'max-height': popOutMenuHeight,
        });

        $('.main-menu-content > ul.menu-content').perfectScrollbar({
          theme:scroll_theme,
        });
      }
    },

    collapse: function($listItem, callback) {
      var $subList = $listItem.children('ul');

      $subList.show().slideUp($.app.nav.config.speed, function() {
        $(this).css('display', '');

        $(this).find('> li').removeClass('is-shown');

        if (callback) {
          callback();
        }

        $.app.nav.container.trigger('collapsed.app.menu');
      });
    },

    expand: function($listItem, callback) {
      var $subList  = $listItem.children('ul');
      var $children = $subList.children('li').addClass('is-hidden');

      $subList.hide().slideDown($.app.nav.config.speed, function() {
        $(this).css('display', '');

        if (callback) {
          callback();
        }

        $.app.nav.container.trigger('expanded.app.menu');
      });

      setTimeout(function() {
        $children.addClass('is-shown');
        $children.removeClass('is-hidden');
      }, 0);
    },

    refresh: function() {
      $.app.nav.container.find('.open').removeClass('open');
    },
  };

})(window, document, jQuery);