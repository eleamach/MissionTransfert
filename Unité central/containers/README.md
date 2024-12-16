# Escape Game
## Repo content

This `docker-compose.yml` file will run :
- home assistant
- moquitto brocker

`setup_and_run_containers.sh` creates the docker volumes subfolders and launch the containers

```bash 
./setup_and_run_containers.sh
```

`setup_and_run_containers.sh` also copy in the docker volume the `mosquitto.conf` file with the following features : 
- listener 1883 0.0.0.0 
- allow_anonymous true

