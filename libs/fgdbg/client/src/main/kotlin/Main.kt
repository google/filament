/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import androidx.compose.material.MaterialTheme
import androidx.compose.desktop.ui.tooling.preview.Preview
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.width
import androidx.compose.material.Button
import androidx.compose.material.ButtonColors
import androidx.compose.material.ButtonDefaults
import androidx.compose.material.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.MutableState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.compose.ui.window.Window
import androidx.compose.ui.window.application
import io.ktor.client.*
import io.ktor.client.plugins.websocket.*
import io.ktor.http.*
import io.ktor.websocket.Frame
import io.ktor.websocket.readText
import kotlinx.coroutines.delay
import org.jetbrains.skia.impl.Log

private enum class ConnectionState { UNKNOWN, CONNECTED, CONNECTING, ERROR }

private data class Connection(
  val host: String = "localhost",
  val port: Int = 8082,
  val path: String = "/",
  val state: MutableState<ConnectionState> = mutableStateOf(ConnectionState.UNKNOWN),
  val reconnectDelay: Int = 5,
)

data class Pass(
  val id: Int,
  val name: String,
  val reads: Set<Int>,
  val writes: Set<Int>,
)

data class Resource(
  val id: Int,
  val name: String,
  val size: Int,
)

fun parseMessage(message: String): Pair<List<Pass>, List<Resource>> {
  Log.debug("Received message: $message")
  val split = message.split('|')
  val passes = split[0].split(',').map { passes ->
    val passInfo = passes.split(":")
    Pass(
      passInfo[0].toInt(),
      passInfo[1],
      passInfo[2].split("-").filter { it.isNotEmpty() }.map { it.toInt() }.toSet(),
      passInfo[3].split("-").filter { it.isNotEmpty() }.map { it.toInt() }.toSet(),
    )
  }
  val resources = split[1].split(',').map {
    val resourceInfo = it.split(':')
    Resource(
      resourceInfo[0].toInt(),
      resourceInfo[1],
      resourceInfo[2].toInt(),
    )
  }
  return passes to resources;
}

@Composable
@Preview
fun App() {
  val connection = remember { Connection() }
  val client = remember { HttpClient { install(WebSockets) } }
  var message by remember { mutableStateOf("") }
  val resources = remember { mutableStateOf(emptyList<Resource>()) }
  val passes = remember { mutableStateOf(emptyList<Pass>()) }

  LaunchedEffect(Unit) {
    while (true) {
      connection.state.value = ConnectionState.CONNECTING
      try {
        client.webSocket(
          HttpMethod.Get, connection.host, connection.port, connection.path
        ) {
          message = "Connected!"
          connection.state.value = ConnectionState.CONNECTED
          while (true) {
            val frame = incoming.receive() as? Frame.Text ?: continue
            val (p, r) = parseMessage(frame.readText())
            passes.value = p
            resources.value = r
          }
        }
      } catch (e: Exception) {
        connection.state.value = ConnectionState.ERROR
        repeat(connection.reconnectDelay) {
          val remaining = connection.reconnectDelay - it
          message = "Connection interrupted, will re-connect in $remaining seconds..."
          delay(1000)
        }
      }
    }
  }

  DisposableEffect(Unit) { onDispose { client.close() } }

  MaterialTheme {
    when (connection.state.value) {
      ConnectionState.CONNECTED -> {
        Column {
          Text(text = "Listening on ${connection.host}:${connection.port} ")
          Grid(passes.value, resources.value)
        }
      }
      ConnectionState.ERROR -> Text(text = message, color = Color.Red)
      else -> Text(connection.state.value.name)
    }
  }
}

@Composable
fun Grid(passes: List<Pass>, resources: List<Resource>) {
  val width = 120.dp
  val height = 40.dp
  Column {
    //val maxNameLength = passes.maxOf { it.name.length }
    // HEADER
    Row(Modifier.height(height)) {
      Spacer(modifier = Modifier.width(width))
      for (pass in passes) Button(modifier = Modifier.width(width), onClick = {}) {
        Text(pass.name)
      }
    }
    // ROWS
    for (resource in resources) {
      Row(Modifier.height(height)) {
        Button(modifier = Modifier.width(width), onClick = {}) { Text(resource.name) }
        // intersections
        for (pass in passes) {
          val writes = pass.writes.contains(resource.id)
          val reads = pass.reads.contains(resource.id)
          val color = when {
            writes && reads -> Color.Magenta
            writes -> Color.Red
            reads -> Color.Green
            else -> Color.LightGray
          }
          Button(
            modifier = Modifier.width(width),
            colors = ButtonDefaults.buttonColors(backgroundColor = color), onClick = {}) {}
        }
      }
    }
  }
}

fun main() = application { Window(onCloseRequest = ::exitApplication) { App() } }
