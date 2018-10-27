/*=========================================================================================
  File Name: app.js
  Description: Template related app JS.
  ----------------------------------------------------------------------------------------
  Item Name: Crypto ICO - Cryptocurrency Website Landing Page HTML + Dashboard Template
  Version: 1.0
  Author: Pixinvent
  Author URL: hhttp://www.themeforest.net/user/pixinvent
==========================================================================================*/

(function(window, document, $) {
    'use strict';
    var $html = $('html');
    var $body = $('body');


    $(window).on('load', function() {
        var rtl;
        var compactMenu = false; // Set it to true, if you want default menu to be compact

        if ($('html').data('textdirection') == 'rtl') {
            rtl = true;
        }

        setTimeout(function() {
            $html.removeClass('loading').addClass('loaded');
        }, 1200);

        $.app.menu.init(compactMenu);

        // Navigation configurations
        var config = {
            speed: 300 // set speed to expand / collpase menu
        };
        if ($.app.nav.initialized === false) {
            $.app.nav.init(config);
        }

        Unison.on('change', function(bp) {
            $.app.menu.change();
        });

        // Tooltip Initialization
        $('[data-toggle="tooltip"]').tooltip({
            container: 'body'
        });

        // Collapsible Card
        $('a[data-action="collapse"]').on('click', function(e) {
            e.preventDefault();
            $(this).closest('.card').children('.card-content').collapse('toggle');
            $(this).closest('.card').find('[data-action="collapse"] i').toggleClass('ft-minus ft-plus');

        });

        // Toggle fullscreen
        $('a[data-action="expand"]').on('click', function(e) {
            e.preventDefault();
            $(this).closest('.card').find('[data-action="expand"] i').toggleClass('ft-maximize ft-minimize');
            $(this).closest('.card').toggleClass('card-fullscreen');
        });

        //  Notifications & messages scrollable
        if ($('.scrollable-container').length > 0) {
            $('.scrollable-container').perfectScrollbar({
                theme: "dark"
            });
        }

        // Reload Card
        $('a[data-action="reload"]').on('click', function() {
            var block_ele = $(this).closest('.card');

            // Block Element
            block_ele.block({
                message: '<div class="ft-refresh-cw icon-spin font-medium-2"></div>',
                timeout: 2000, //unblock after 2 seconds
                overlayCSS: {
                    backgroundColor: '#FFF',
                    cursor: 'wait',
                },
                css: {
                    border: 0,
                    padding: 0,
                    backgroundColor: 'none'
                }
            });
        });

        // Close Card
        $('a[data-action="close"]').on('click', function() {
            $(this).closest('.card').removeClass().slideUp('fast');
        });

        // Match the height of each card in a row
        setTimeout(function() {
            $('.row.match-height').each(function() {
                $(this).find('.card').not('.card .card').matchHeight(); // Not .card .card prevents collapsible cards from taking height
            });
        }, 500);


        $('.card .heading-elements a[data-action="collapse"]').on('click', function() {
            var $this = $(this),
                card = $this.closest('.card');
            var cardHeight;

            if (parseInt(card[0].style.height, 10) > 0) {
                cardHeight = card.css('height');
                card.css('height', '').attr('data-height', cardHeight);
            } else {
                if (card.data('height')) {
                    cardHeight = card.data('height');
                    card.css('height', cardHeight).attr('data-height', '');
                }
            }
        });

        // Add open class to parent list item if subitem is active except compact menu
        var menuType = $body.data('menu');
        if (menuType != 'vertical-compact-menu' && menuType != 'horizontal-menu' && compactMenu === false) {
            if ($body.data('menu') == 'vertical-menu-modern') {
                if (localStorage.getItem("menuLocked") === "true") {
                    $(".main-menu-content").find('li.active').parents('li').addClass('open');
                }
            } else {
                $(".main-menu-content").find('li.active').parents('li').addClass('open');
            }
        }
        if (menuType == 'vertical-compact-menu' || menuType == 'horizontal-menu') {
            $(".main-menu-content").find('li.active').parents('li:not(.nav-item)').addClass('open');
            $(".main-menu-content").find('li.active').parents('li').addClass('active');
        }

        //card heading actions buttons small screen support
        $(".heading-elements-toggle").on("click", function() {
            $(this).parent().children(".heading-elements").toggleClass("visible");
        });

        //  Dynamic height for the chartjs div for the chart animations to work
        var chartjsDiv = $('.chartjs'),
            canvasHeight = chartjsDiv.children('canvas').attr('height');
        chartjsDiv.css('height', canvasHeight);


        $('.nav-link-search').on('click', function() {
            var $this = $(this),
                searchInput = $(this).siblings('.search-input');

            if (searchInput.hasClass('open')) {
                searchInput.removeClass('open');
            } else {
                searchInput.addClass('open');
            }
        });
    });


    $(document).on('click', '.menu-toggle, .modern-nav-toggle', function(e) {
        e.preventDefault();

        // Toggle menu
        $.app.menu.toggle();

        setTimeout(function() {
            $(window).trigger("resize");
        }, 200);

        if ($('#collapsed-sidebar').length > 0) {
            setTimeout(function() {
                if ($body.hasClass('menu-expanded') || $body.hasClass('menu-open')) {
                    $('#collapsed-sidebar').prop('checked', false);
                } else {
                    $('#collapsed-sidebar').prop('checked', true);
                }
            }, 1000);
        }

        return false;
    });

    $(document).on('click', '.open-navbar-container', function(e) {

        var currentBreakpoint = Unison.fetch.now();

        // Init drilldown on small screen
        $.app.menu.drillDownMenu(currentBreakpoint.name);

        // return false;
    });

    $(document).on('click', '.main-menu-footer .footer-toggle', function(e) {
        e.preventDefault();
        $(this).find('i').toggleClass('pe-is-i-angle-down pe-is-i-angle-up');
        $('.main-menu-footer').toggleClass('footer-close footer-open');
        return false;
    });

    // Add Children Class
    $('.navigation').find('li').has('ul').addClass('has-sub');


    // Update manual scroller when window is resized
    $(window).resize(function() {
        $.app.menu.manualScroller.updateHeight();
    });


    // Add navbar shadow on scroll
    $(window).scroll(function() {
        // console.log($(window).scrollTop());
        if ($(window).scrollTop() > 60) {
            $('.header-navbar').addClass('navbar-shadow');
            $('.header-navbar').css('opacity', '0.97');
        }
        if ($(window).scrollTop() < 60) {
            $('.header-navbar').removeClass('navbar-shadow');
            $('.header-navbar').css('opacity', '1');
        }
    });

})(window, document, jQuery);