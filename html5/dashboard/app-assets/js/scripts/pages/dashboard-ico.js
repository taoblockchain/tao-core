/*=========================================================================================
    File Name: dashboard-ecommerce.js
    Description: dashboard-ecommerce
    ----------------------------------------------------------------------------------------
    Item Name: Crypto ICO - Cryptocurrency Website Landing Page HTML + Dashboard Template
    Version: 1.0
    Author: PIXINVENT
    Author URL: http://www.themeforest.net/user/pixinvent
==========================================================================================*/

$(window).on("load", function() {

    // ICO TOKEN (SUPPLY & DEMAND)
    var verticalBar3 = new Chartist.Bar('#ico-token-supply-demand-chart', {
        labels: ['Q1 2018', 'Q2 2018', 'Q3 2018', 'Q4 2018', 'Q1 2019', 'Q2 2019', 'Q3 2019', 'Q4 2019', 'Q1 2020', 'Q2 2020', 'Q3 2020', 'Q4 2020'],
        series: [
            [4000, 7000, 5000, 2500, 5200, 4400, 7000, 4200, 7500, 2000, 3000, 5000],
        ]
    }, {
        axisY: {
            labelInterpolationFnc: function(value) {
                return (value / 1000) + 'k';
            },
            scaleMinSpace: 40,
        },
        axisX: {
            showGrid: false,
            labelInterpolationFnc: function(value, index) {
                return value;
            }
        },
        plugins: [
            Chartist.plugins.tooltip({
                appendToBody: true,
                pointClass: 'ct-point'
            })
        ]
    });
    verticalBar3.on('draw', function(data) {
        if (data.type === 'bar') {
            data.element.attr({
                style: 'stroke-width: 25px',
                y1: 250,
                x1: data.x1 + 0.001
            });
            data.group.append(new Chartist.Svg('circle', {
                cx: data.x2,
                cy: data.y2,
                r: 12
            }, 'ct-slice-pie'));
        }
    });
    verticalBar3.on('created', function(data) {
        var defs = data.svg.querySelector('defs') || data.svg.elem('defs');
        defs.elem('linearGradient', {
            id: 'barGradient3',
            x1: 0,
            y1: 0,
            x2: 0,
            y2: 1
        }).elem('stop', {
            offset: 0,
            'stop-color': 'rgb(28, 120, 255)'            
        }).parent().elem('stop', {
            offset: 1,
            'stop-color': 'rgb(41, 188, 253)'
        });
        return defs;
    });


    // TOKEN DISTRIBUTION
    var chart = new Chartist.Pie('#token-distribution-chart', {
        // series: [10, 20, 50, 20, 5, 50, 15],
        series: [{
                "name": "Crowdsale",
                "className": "ct-crowdsale",
                "value": 41
            },
            {
                "name": "Team",
                "className": "ct-team",
                "value": 18
            },
            {
                "name": "Advisors",
                "className": "ct-advisors",
                "value": 15
            },
            {
                "name": "Project reserve",
                "className": "ct-project-advisors",
                "value": 10
            },
            {
                "name": "Masternodes",
                "className": "ct-masternodes",
                "value": 8
            },
            {
                "name": "Program",
                "className": "ct-program",
                "value": 8
            },
        ],

        labels: ["Crowdsale", "Team", "Advisors", "Project reserve", "Masternodes", "Program"]
    }, {
        donut: true,
        startAngle: 310,
        donutSolid: true,
        donutWidth: 30,
        labelInterpolationFnc: function(value) {
            var total = chart.data.series.reduce(function(prev, series) {
                return prev + series.value;
            }, 0);
            return total + '%';
        }
    });

    chart.on('created', function() {
        var defs = chart.svg.elem('defs');

        defs.elem('linearGradient', {
            id: 'ct-crowdsale',
            x1: 0,
            y1: 1,
            x2: 0,
            y2: 0
        }).elem('stop', {
            offset: 0,
            'stop-color': '#2191ff'
        }).parent().elem('stop', {
            offset: 1,
            'stop-color': '#2abbfe'
        });

        defs.elem('linearGradient', {
            id: 'ct-team',
            x1: 0,
            y1: 1,
            x2: 0,
            y2: 0
        }).elem('stop', {
            offset: 0,
            'stop-color': '#892ffe'
        }).parent().elem('stop', {
            offset: 1,
            'stop-color': '#c37bfe'
        });

        defs.elem('linearGradient', {
            id: 'ct-advisors',
            x1: 0,
            y1: 1,
            x2: 0,
            y2: 0
        }).elem('stop', {
            offset: 0,
            'stop-color': '#6aecae'
        }).parent().elem('stop', {
            offset: 1,
            'stop-color': '#39d98c'
        });

        defs.elem('linearGradient', {
            id: 'ct-project-advisors',
            x1: 0,
            y1: 1,
            x2: 0,
            y2: 0
        }).elem('stop', {
            offset: 0,
            'stop-color': '#f19686'
        }).parent().elem('stop', {
            offset: 1,
            'stop-color': '#e85d44'
        });

        defs.elem('linearGradient', {
            id: 'ct-masternodes',
            x1: 0,
            y1: 1,
            x2: 0,
            y2: 0
        }).elem('stop', {
            offset: 0,
            'stop-color': '#e67ea5'
        }).parent().elem('stop', {
            offset: 1,
            'stop-color': '#fa679d'
        });

        defs.elem('linearGradient', {
            id: 'ct-program',
            x1: 0,
            y1: 1,
            x2: 0,
            y2: 0
        }).elem('stop', {
            offset: 0,
            'stop-color': '#99f3f3'
        }).parent().elem('stop', {
            offset: 1,
            'stop-color': '#33d4d8'
        });
    });
    chart.on('draw', function(data) {
        if (data.type === 'label') {
            if (data.index === 0) {
                data.element.attr({
                    dx: data.element.root().width() / 2,
                    dy: data.element.root().height() / 2
                });
            } else {
                data.element.remove();
            }
        }
    });
});