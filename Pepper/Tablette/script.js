

$(document).ready(function () {
    session = new QiSession();

    session.service("ALMemory").done(function(ALMemory) {

        ALMemory.subscriber("SimpleWeb/Page/Start").done(function(subscriber) {
            subscriber.signal.connect(function() {
                $('#section_start').addClass('active').siblings('.section').removeClass('active');
            });
        });
    
        ALMemory.subscriber("SimpleWeb/Page/Progress0").done(function(subscriber) {
            subscriber.signal.connect(function() {
                $('#section_0').addClass('active').siblings('.section').removeClass('active');
            });
        });
    
        ALMemory.subscriber("SimpleWeb/Page/Progress33").done(function(subscriber) {
            subscriber.signal.connect(function() {
                $('#section_33').addClass('active').siblings('.section').removeClass('active');
            });
        });

        ALMemory.subscriber("SimpleWeb/Page/Progress66").done(function(subscriber) {
            subscriber.signal.connect(function() {
                $('#section_66').addClass('active').siblings('.section').removeClass('active');
            });
        });

        ALMemory.subscriber("SimpleWeb/Page/Progress99").done(function(subscriber) {
            subscriber.signal.connect(function() {
                $('#section_99').addClass('active').siblings('.section').removeClass('active');
            });
        });

        ALMemory.subscriber("SimpleWeb/Page/Code").done(function(subscriber) {
            subscriber.signal.connect(function() {
                $('#section_code').addClass('active').siblings('.section').removeClass('active');
            });
        });

        ALMemory.subscriber("SimpleWeb/Page/Reussi").done(function(subscriber) {
            subscriber.signal.connect(function() {
                $('#section_final').addClass('active').siblings('.section').removeClass('active');
            });
        });
        
        ALMemory.subscriber("SimpleWeb/Page/Perdu").done(function(subscriber) {
            subscriber.signal.connect(function() {
                $('#section_perdu').addClass('active').siblings('.section').removeClass('active');
            });
        });

        ALMemory.subscriber("SimpleWeb/Page/FinalButton").done(function(subscriber) {
            subscriber.signal.connect(function() {
                $('#section_bouton').addClass('active').siblings('.section').removeClass('active');
            });
        });
                
        function subscribeToSection(sectionId) {
            ALMemory.subscriber("SimpleWeb/Page/" + sectionId).done(function(subscriber) {
                subscriber.signal.connect(function() {
                    $('#' + sectionId).addClass('active').siblings('.section').removeClass('active');
                });
            });
        }

        subscribeToSection('Progress0'); 
        subscribeToSection('Progress33'); 
        subscribeToSection('Progress66'); 
        subscribeToSection('Progress99'); 
        subscribeToSection('Code');      

        $('.custom-button').click(function() {
            $('#section_code').addClass('active').siblings('.section').removeClass('active');
            console.log("click");
        });
        

    });
    
});