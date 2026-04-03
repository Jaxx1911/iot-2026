from contextlib import asynccontextmanager
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from .routes import router
from . import mqtt_client


@asynccontextmanager
async def lifespan(app: FastAPI):
    mqtt_client.start()
    yield


app = FastAPI(
    title="BMS Backend",
    description="Battery Management System API for LiFePO4 4S renewable energy storage",
    version="1.0.0",
    lifespan=lifespan,
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(router, prefix="/api")
