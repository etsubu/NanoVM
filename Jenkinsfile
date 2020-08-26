pipeline {
    agent {
        dockerfile true
    }
    stages {
        stage("Presteps") {
            steps {
                sh 'cmake .'
            }
        }
        stage('Build') {
            steps {
                sh 'make'
            }
        }
        stage('Test') {
            steps {
                sh 'make test'
            }
        }
    }
}

