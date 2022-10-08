// Copyright 2000-2021 JetBrains s.r.o. and contributors. Use of this source code is governed by the Apache 2.0 license that can be found in the LICENSE file.
import androidx.compose.material.MaterialTheme
import androidx.compose.desktop.ui.tooling.preview.Preview
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.height
import androidx.compose.material.Button
import androidx.compose.material.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.DisposableEffectResult
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.compose.ui.window.Window
import androidx.compose.ui.window.application
import io.ktor.client.*
import io.ktor.client.plugins.websocket.*
import io.ktor.http.*
import io.ktor.websocket.*
import io.ktor.util.*
import kotlinx.coroutines.*

@Composable
@Preview
fun App() {

  val host = remember { "localhost" }
  val port = remember { 8082 }
  val client = remember { HttpClient { install(WebSockets) } }
  var text by remember { mutableStateOf("") }

  LaunchedEffect(Unit) {
    client.webSocket(method = HttpMethod.Get, host = host, port = port, path = "/") {
      while (true) {
        val message = incoming.receive() as? Frame.Text ?: continue
        text = message.readText()
      }
    }
  }

  DisposableEffect(Unit) {
    onDispose {
      client.close()
    }
  }

  MaterialTheme {
    Row(Modifier.height(40.dp)) {
      Text(modifier = Modifier.align(Alignment.CenterVertically),
           text = "Listening on $host:$port: ")
      Button(onClick = {}) { Text(text) }
    }
  }
}

fun main() = application {
  Window(onCloseRequest = ::exitApplication) {
    App()
  }
}
