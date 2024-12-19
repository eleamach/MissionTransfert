

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

        //TODO: publisher

                // Fonction pour abonner un signal pour afficher la section
        function subscribeToSection(sectionId) {
            // Abonnez-vous au signal associé à chaque section
            ALMemory.subscriber("SimpleWeb/Page/" + sectionId).done(function(subscriber) {
                subscriber.signal.connect(function() {
                    // Afficher la section correspondante
                    $('#' + sectionId).addClass('active').siblings('.section').removeClass('active');
                });
            });
        }

        // Abonnez-vous aux différentes sections
        subscribeToSection('Progress0');  // Section pour 0% de progression
        subscribeToSection('Progress33'); // Section pour 33% de progression
        subscribeToSection('Progress66'); // Section pour 66% de progression
        subscribeToSection('Progress99'); // Section pour 99% de progression
        subscribeToSection('Code');      // Section pour le code

        $('.custom-button').click(function() {
            $('#section_code').addClass('active').siblings('.section').removeClass('active');
            console.log("click");
        });
        

    });
    
});