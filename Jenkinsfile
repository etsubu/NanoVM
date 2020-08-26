pipeline {
    agent {
        dockerfile true
    }
    triggers { cron(env.BRANCH_NAME == "master" ? "H 4 * * *" : "") }
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
    post {
        cleanup {
            cleanWs()
        }
    }
}

